#! /usr/bin/python

import os
import os.path
import re
import sys

ErrorPattern = re.compile('([A-Z]{3}_ERROR_[A-Z_0-9]+)\s+=?\s*\(?([A-Z_0-9-][A-Z_0-9-+ ]+[A-Z_0-9])')
FilePatternH = re.compile('^.*\.h$')
FilePatternC = re.compile('^.*\.(c|cpp)$')

Errors = {}
Codes = {}

ERROR_MAP_HEADER = """
/*****************************************************************
|
|   Neptune - Result Code Map
|
|   This file is automatically generated by a script, do not edit!
|
| Copyright (c) 2002-2008, Axiomatic Systems, LLC.
| All rights reserved.
|
| Redistribution and use in source and binary forms, with or without
| modification, are permitted provided that the following conditions are met:
|     * Redistributions of source code must retain the above copyright
|       notice, this list of conditions and the following disclaimer.
|     * Redistributions in binary form must reproduce the above copyright
|       notice, this list of conditions and the following disclaimer in the
|       documentation and/or other materials provided with the distribution.
|     * Neither the name of Axiomatic Systems nor the
|       names of its contributors may be used to endorse or promote products
|       derived from this software without specific prior written permission.
|
| THIS SOFTWARE IS PROVIDED BY AXIOMATIC SYSTEMS ''AS IS'' AND ANY
| EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
| WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
| DISCLAIMED. IN NO EVENT SHALL AXIOMATIC SYSTEMS BE LIABLE FOR ANY
| DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
| (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
| LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
| ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
| (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
| SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
|
 ****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"

/*----------------------------------------------------------------------
|   NPT_ResultText
+---------------------------------------------------------------------*/
const char*
NPT_ResultText(NPT_Result result)
{
    switch (result) {
        case NPT_SUCCESS: return "SUCCESS";
        case NPT_FAILURE: return "FAILURE";
"""

ERROR_MAP_FOOTER = """
        default: return "UNKNOWN";
    }
}
"""

def ResolveErrors():
    keep_going = True
    while keep_going:
        keep_going = False
        for key in Errors.keys():
            value = Errors[key]
            if type(value) is str:
                elements = [x.strip() for x in value.split('-')]
                if len(elements[0]) == 0:
                    first = 0
                else:
                    first = elements[0]
                if Errors.has_key(first):
                    first = Errors[first]
                if not type(first) is str:
                    second = int(elements[1])
                    Errors[key] = first-second
                    keep_going = True
            
    
def AnalyzeErrorCodes(file):
    input = open(file)
    for line in input.readlines():
        m = ErrorPattern.search(line)
        if m:
            Errors[m.group(1)] = m.group(2)
    input.close()
    
def ScanErrorCodes(top):
    print ERROR_MAP_HEADER
    
    for root, dirs, files in os.walk(top):
        for file in files:
            if FilePatternH.match(file):
                 AnalyzeErrorCodes(os.path.join(root, file))
        
    ResolveErrors()
    for key in Errors:
        #print key,"==>",Errors[key]
        if (key.find("ERROR_BASE") > 1): continue
        if Codes.has_key(Errors[key]):
            raise "duplicate error code: "+ str(key) +" --> " + str(Errors[key]) + "=" + Codes[Errors[key]]
        Codes[Errors[key]] = key
        
    sorted_keys = Codes.keys()
    sorted_keys.sort()
    sorted_keys.reverse()
    last = 0
    for code in sorted_keys:
        #if code != last-1:
        #    print 
        print '        case %s: return "%s";' % (Codes[code],Codes[code])
        last = code
    
    print ERROR_MAP_FOOTER
    
####################################################
# main
####################################################
sys.argv.reverse()
sys.argv.pop()
action = None
top = None
while len(sys.argv):
    arg = sys.argv.pop()
    if top == None:
        top = arg
    else:
        raise "unexpected argument " + arg

if top is None:
    print "GenErrorMap.py <directory-root>"
    sys.exit(1)

ScanErrorCodes(top)
    
    
