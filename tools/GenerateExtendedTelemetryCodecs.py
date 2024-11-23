#!env -S python -u

import json
import math
import sys
import os


#####################################################################
# Utility
#####################################################################

class StrAccumulator:
    def __init__(self):
        self.str = ''
        self.indent = 0

    def A(self, appendStr):
        self.str += ' ' * self.indent + appendStr + '\n'

    def IncrIndent(self):
        self.indent += 4

    def DecrIndent(self):
        self.indent -= 4

    def Get(self):
        return self.str

# fix lines with same features but different spacing between them.
# requires all lines to have same number of separated elements.
def Align(strList, splitCharList = [" "]):
    for splitChar in splitCharList:
        # init array of max lens to the first string in the list
        maxLenList = [0] * len(strList[0].split(splitChar))

        print(f"first line len: {len(maxLenList)}")

        # for each string, find out max len of each component
        for str in strList:
            strPartList = str.split(splitChar)

            print(f"strPartList len: {len(strPartList)}")

            i = 0
            for strPart in strPartList:
                strPart = strPart.lstrip()
                strPartLen = len(strPart)

                if strPartLen > maxLenList[i]:
                    maxLenList[i] = strPartLen

                i += 1

        # break strings up and pad them to be the max len
        strListNew = []
        for str in strList:
            strPartList = str.split(splitChar)
            strPartListNew = []

            i = 0
            for strPart in strPartList:
                strPart = strPart.lstrip()
                strPart = strPart.ljust(maxLenList[i])
                strPartListNew.append(strPart)

                i += 1
            
            strNew = splitChar.join(strPartListNew)

            strListNew.append(strNew)
        
        strList = strListNew

    return strListNew


#####################################################################
# Codec Logic and Code Generation
#####################################################################

class WsprCodecMaker:
    def __init__(self):
        self.debug = False
        self.codec = ""
        self.json = {}
        self.errList = []
        self.SetCodecDefFragment("MyMessageType", "")

    def SetDebug(self, debug):
        self.debug = debug

    def SetCodecDefFragment(self, msgName, codecFragment):
        finalFieldFragment = '''
        { "name": "HdrType",          "unit": "Enum", "lowValue": 0, "highValue": 15, "stepSize": 1 },
        { "name": "HdrSlot",          "unit": "Enum", "lowValue": 0, "highValue":  3, "stepSize": 1 },
        { "name": "HdrRESERVED",      "unit": "Enum", "lowValue": 0, "highValue":  3, "stepSize": 1 },
        { "name": "HdrTelemetryType", "unit": "Enum", "lowValue": 0, "highValue":  1, "stepSize": 1 }
        '''

        codec = f'''
{{
    "name": "{msgName}",
    "fieldList": [
{codecFragment} {finalFieldFragment}]
}}'''
        ok = self.SetCodecDef(codec)
        return ok

    def SetCodecDef(self, codec):
        self.codec = codec
        ok = self.ParseCodecDef(self.codec)

        if ok:
            self.Calculate()

        return ok

    def GetErrList(self):
        return self.errList

    def ResetErrList(self):
        self.errList = []

    def AddErr(self, err):
        self.errList.append(err)

        if self.debug:
            print(err)

    def ParseCodecDef(self, codec):
        ok = True

        if self.debug:
            print(codec)

        self.ResetErrList()

        try:
            import json
            self.json = json.loads(codec)

            if "name" not in self.json:
                ok = False
                self.AddErr('No "name" property for codec')
            elif "fieldList" not in self.json:
                ok = False
                self.AddErr('No "fieldList" property for codec')
            else:
                for field in self.json['fieldList']:
                    if "name" not in field:
                        ok = False
                        self.AddErr('No "name" property in field')
                    else:
                        propList = [
                            "unit",
                            "lowValue",
                            "highValue",
                            "stepSize",
                        ]

                        for prop in propList:
                            if prop not in field:
                                ok = False
                                self.AddErr(f'No "{prop}" property in field({field["name"]})')

                        if ok:
                            if field['lowValue'] >= field['highValue']:
                                ok = False
                                self.AddErr(f'Field({field["name"]}) lowValue({field["lowValue"]}) must be less than highValue({field["highValue"]})')

                        if ok:
                            stepCount = (field['highValue'] - field['lowValue']) / field['stepSize']

                            if not stepCount.is_integer():
                                ok = False
                                err = f'Field({field["name"]}) stepSize({field["stepSize"]}) does not evenly divide the low({field["lowValue"]})-to-high({field["highValue"]}) range.'
                                factorList = []

                                if isinstance(field['lowValue'], int) and isinstance(field['highValue'], int):
                                    for stepSize in range(1, (field['highValue'] - field['lowValue'])): # 2):
                                        stepCountNew = (field['highValue'] - field['lowValue']) / stepSize

                                        if stepCountNew.is_integer():
                                            factorList.append(stepSize)

                                    if factorList:
                                        err += f'\n    Whole integer steps are: {", ".join(map(str, factorList))}.'

                                self.AddErr(err)
        except Exception as e:
            ok = False
            self.AddErr(e)

        return ok

    def GetLastErr(self):
        return self.lastErr

    def Calculate(self):
        bitsLast = 0
        bitsSum = 0
        for field in self.json['fieldList']:
            field['NumValues'] = int((field['highValue'] - field['lowValue']) // field['stepSize']) + 1
            field['Bits'] = math.log2(field['NumValues'])
            field['BitsSum'] = field['Bits'] + bitsSum

            bitsLast = field['Bits']
            bitsSum += field['Bits']

        if self.debug:
            print(self.json['fieldList'])

    def GetCodec(self):
        c = self.GenerateCodecClassDef()
        if self.debug:
            print(c)
        
        return c

    def GenerateCodecClassDef(self):
        a = StrAccumulator()

        a.A('#pragma once')
        a.A('')
        a.A('#include "WSPRMessage.h"')
        a.A('')
        a.A('')
        a.A(f'class {self.json["name"]}Codec')
        a.A('{')
        a.A('public:')
        a.A('')

        # Constructor
        a.IncrIndent()
        a.A(f'{self.json["name"]}Codec()')
        a.A('{')
        a.IncrIndent()
        a.A('Reset();')
        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        a.A('')

        # Set id13
        a.IncrIndent()
        a.A('SetId13(id13)')
        a.A('{')
        a.IncrIndent()
        a.A('id13 = id13;')
        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        a.A('')

        # Get id13
        a.IncrIndent()
        a.A('inline <type> GetId13(id13) const')
        a.A('{')
        a.IncrIndent()
        a.A('return id13;')
        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        # Setters / Getters
        for field in self.json['fieldList']:
            a.A('')

            # Setter
            a.IncrIndent()
            a.A(f'Set{field["name"]}{field["unit"]}(inputVal)')
            a.A('{')
            a.IncrIndent()

            a.A(f'let val = inputVal ?? {field["lowValue"]};')
            a.A('')
            a.A(f'if (val < {field["lowValue"]}) {{ val = {field["lowValue"]}; }}')
            a.A(f'else if (val > {field["highValue"]}) {{ val = {field["highValue"]}; }}')
            a.A('')
            a.A(f'{field["name"]} = val;')

            a.DecrIndent()
            a.A('}')
            a.DecrIndent()

            a.A('')

            # Getter
            a.IncrIndent()
            a.A(f'inline <type> Get{field["name"]}{field["unit"]}() const')
            a.A('{')
            a.IncrIndent()

            a.A(f'return {field["name"]};')

            a.DecrIndent()
            a.A('}')
            a.DecrIndent()

            a.A('')

            # Encoded Number Getter
            a.IncrIndent()
            a.A(f'inline <type> Get{field["name"]}{field["unit"]}Number() const')
            a.A('{')
            a.IncrIndent()

            a.A(f'let retVal = null;')
            a.A('')
            a.A(f'retVal = ((Get{field["name"]}{field["unit"]}() - {field["lowValue"]}) / {field["stepSize"]});')
            a.A(f'retVal = Math.round(retVal);')
            a.A('')
            a.A(f'return retVal;')

            a.DecrIndent()
            a.A('}')
            a.DecrIndent()

        a.A('')

        # Encode

        # arrange application fields in reverse order
        # but ensure the original order of header fields.
        # this allows decode to pull out the "first" application field
        # consistently, even if the fields after it
        # change, are added, or revmoed.
        # this isn't an expected feature, but a good feature as it protects
        # legacy data in the event of future change as much as possible.
        fieldEncodeList = self.json['fieldList'][:]
        fieldListApp = []
        fieldListHdr = []

        for field in fieldEncodeList:
            if field['name'][:3] == "Hdr":
                fieldListHdr.append(field)
            else:
                fieldListApp.append(field)

        # reverse the application fields in-place
        fieldListApp.reverse()

        # re-make the field list
        fieldEncodeList = []
        for field in fieldListApp:
            fieldEncodeList.append(field)

        for field in fieldListHdr:
            fieldEncodeList.append(field)

        a.IncrIndent()
        a.A('Encode()')
        a.A('{')
        a.IncrIndent()

        a.A('uint32_t val = 0;')
        a.A('')

        a.A('// combine field values')
        strList = []
        for field in fieldEncodeList:
            strList.append(f'val *= {field["NumValues"]}; val += Get{field["name"]}{field["unit"]}Number();')

        for strNew in Align(strList, ["; "]):
            a.A(strNew)

        a.A('')

        a.A('// encode into power')
        a.A('uint8_t powerVal = val % 19; val = Math.floor(val / 19);')
        a.A('uint8_t powerDbm = WSPR::GetPowerDbmList()[powerVal];;')
        a.A('')
        a.A('// encode into grid')
        a.A('uint8_t g4Val = val % 10; val = Math.floor(val / 10);')
        a.A('uint8_t g3Val = val % 10; val = Math.floor(val / 10);')
        a.A('uint8_t g2Val = val % 18; val = Math.floor(val / 18);')
        a.A('uint8_t g1Val = val % 18; val = Math.floor(val / 18);')
        a.A('')
        a.A('char g1 = String.fromCharCode("A".charCodeAt(0) + g1Val);')
        a.A('char g2 = String.fromCharCode("A".charCodeAt(0) + g2Val);')
        a.A('char g3 = String.fromCharCode("0".charCodeAt(0) + g3Val);')
        a.A('char g4 = String.fromCharCode("0".charCodeAt(0) + g4Val);')
        a.A('let grid = g1 + g2 + g3 + g4;')
        a.A('')
        a.A('// encode into callsign')
        a.A('uint8_t id6Val = val % 26; val = Math.floor(val / 26);')
        a.A('uint8_t id5Val = val % 26; val = Math.floor(val / 26);')
        a.A('uint8_t id4Val = val % 26; val = Math.floor(val / 26);')
        a.A('uint8_t id2Val = val % 36; val = Math.floor(val / 36);')
        a.A('')
        a.A('char id2 = wsprEncoded.EncodeBase36(id2Val);')
        a.A('char id4 = String.fromCharCode("A".charCodeAt(0) + id4Val);')
        a.A('char id5 = String.fromCharCode("A".charCodeAt(0) + id5Val);')
        a.A('char id6 = String.fromCharCode("A".charCodeAt(0) + id6Val);')
        a.A('let call = id13.at(0) + id2 + id13.at(1) + id4 + id5 + id6;')
        a.A('')
        a.A('// capture results')
        a.A('call     = call;')
        a.A('grid     = grid;')
        a.A('powerDbm = powerDbm;')

        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        a.A('')

        # Decode

        # get an entire-list reversed copy of the encoded field order
        fieldDecodeList = list(reversed(fieldEncodeList))

        a.IncrIndent()
        a.A('Decode()')
        a.A('{')
        a.IncrIndent()
        a.A('// pull in inputs')
        a.A('let call     = GetCall();')
        a.A('let grid     = GetGrid();')
        a.A('let powerDbm = GetPowerDbm();')
        a.A('')
        a.A('// break call down')
        a.A('let id2Val = wsprEncoded.DecodeBase36(call.charAt(1));')
        a.A('let id4Val = call.charAt(3).charCodeAt(0) - "A".charCodeAt(0);')
        a.A('let id5Val = call.charAt(4).charCodeAt(0) - "A".charCodeAt(0);')
        a.A('let id6Val = call.charAt(5).charCodeAt(0) - "A".charCodeAt(0);')
        a.A('')
        a.A('// break grid down')
        a.A('let g1Val = grid.charAt(0).charCodeAt(0) - "A".charCodeAt(0);')
        a.A('let g2Val = grid.charAt(1).charCodeAt(0) - "A".charCodeAt(0);')
        a.A('let g3Val = grid.charAt(2).charCodeAt(0) - "0".charCodeAt(0);')
        a.A('let g4Val = grid.charAt(3).charCodeAt(0) - "0".charCodeAt(0);')
        a.A('')
        a.A('// break power down')
        a.A('let powerVal = wsprEncoded.DecodePowerToNum(powerDbm);')
        a.A('')
        a.A('// combine values into single integer')
        a.A('uint32_t val = 0;')
        a.A('val *= 36; val += id2Val;')
        a.A('val *= 26; val += id4Val;   // spaces aren\'t used, so 26 not 27')
        a.A('val *= 26; val += id5Val;   // spaces aren\'t used, so 26 not 27')
        a.A('val *= 26; val += id6Val;   // spaces aren\'t used, so 26 not 27')
        a.A('val *= 18; val += g1Val;')
        a.A('val *= 18; val += g2Val;')
        a.A('val *= 10; val += g3Val;')
        a.A('val *= 10; val += g4Val;')
        a.A('val *= 19; val += powerVal;')
        a.A('')
        a.A('// extract field values')

        strList = []
        for field in fieldDecodeList:
            strList.append(f'Set{field["name"]}{field["unit"]}({field["lowValue"]} + ((val % {field["NumValues"]}) * {field["stepSize"]})); val /= {field["NumValues"]};')
        for str in Align(strList, ["(", "+ ", "* ", "))", "(val "]):
            a.A(str)

        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        a.A('')

        # SetCall
        a.IncrIndent()
        a.A('SetCall(inputVal)')
        a.A('{')
        a.IncrIndent()
        a.A('call = inputVal;')
        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        a.A('')

        # GetCall
        a.IncrIndent()
        a.A('inline <type> GetCall() const')
        a.A('{')
        a.IncrIndent()
        a.A('return call;')
        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        a.A('')

        # SetGrid
        a.IncrIndent()
        a.A('SetGrid(inputVal)')
        a.A('{')
        a.IncrIndent()
        a.A('grid = inputVal;')
        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        a.A('')

        # GetGrid
        a.IncrIndent()
        a.A('inline <type> GetGrid() const')
        a.A('{')
        a.IncrIndent()
        a.A('return grid;')
        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        a.A('')

        # SetPowerDbm
        a.IncrIndent()
        a.A('SetPowerDbm(inputVal)')
        a.A('{')
        a.IncrIndent()
        a.A('powerDbm = inputVal;')
        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        a.A('')

        # GetPowerDbm
        a.IncrIndent()
        a.A('inline <type> GetPowerDbm() const')
        a.A('{')
        a.IncrIndent()
        a.A('return parseInt(powerDbm);')
        a.DecrIndent()
        a.A('}')
        a.DecrIndent()

        a.A('')
        a.A('')

        a.A('private:')

        a.A('')

        # Reset
        a.IncrIndent()
        a.A('void Reset()')
        a.A('{')
        a.IncrIndent()
        a.A('call     = "0A0AAA";')
        a.A('grid     = "AA00";')
        a.A('powerDbm = 0;')
        a.A('')
        a.A('id13 = "00";')
        a.A('')
        for field in self.json['fieldList']:
            a.A(f'{field["name"]} = {field["lowValue"]};')
        a.DecrIndent()
        a.A('}')

        a.A('')
        a.A('')

        a.DecrIndent()
        a.A('private:')
        a.IncrIndent()

        a.A('')

        # member variables
        a.A('inline <type> call;')
        a.A('inline <type> grid;')
        a.A('inline <type> powerDbm;')
        a.A('')
        a.A('inline <type> id13;')
        a.A('')
        for field in self.json['fieldList']:
            a.A(f'inline <type> {field["name"]};')

        a.DecrIndent()

        a.DecrIndent()
        a.A('}')

        c = a.Get()

        return c



#####################################################################
# File Processing
#####################################################################

def ProcessCodecDef(fieldDefFile):
    retVal = True

    try:
        f = open(fieldDefFile, "r")
        fieldDefStr = f.read()
        f.close()

        # print(fieldDefStr)
    except Exception as e:
        retVal = False

        print(f'ERR: Could not open Field Def file "{fieldDefFile}": {e}')
    
    jsonObj = {}

    if retVal:
        codecMaker = WsprCodecMaker()
        # codecMaker.SetDebug(True)

        retVal = codecMaker.SetCodecDefFragment("MyMessageType", fieldDefStr)

        if retVal:
            # print()
            # print()
            # print()
            # print()
            # print()
            cppCode = codecMaker.GetCodec()
            print(cppCode)

    return retVal


#####################################################################
# Main
#####################################################################

def Main():
    if len(sys.argv) < 2 or (len(sys.argv) >= 1 and sys.argv[1] == "--help"):
        print("Usage: %s <codecDef.json>" % (os.path.basename(sys.argv[0])))
        sys.exit(-1)

    codecDefJsonFile = sys.argv[1]

    ProcessCodecDef(codecDefJsonFile)

    return 0

sys.exit(Main())
