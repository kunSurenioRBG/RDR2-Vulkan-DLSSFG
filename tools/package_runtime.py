"""Package version 1.0 using a complete compatible runtime template."""
from pathlib import Path
import argparse
import hashlib
import zipfile
import re

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--template',type=Path,required=True)
parser.add_argument('--output',type=Path,required=True)
parser.add_argument('--asi',type=Path,default=Path(__file__).resolve().parents[1]/'RDR2VulkanFG.asi')
parser.add_argument('--interposer',type=Path)
parser.add_argument('--diagnostics',type=Path)
parser.add_argument('--core',type=Path)
args=parser.parse_args()
if args.output.exists() or args.output.resolve()==args.template.resolve():
    parser.error('Choose a new output path; the template and existing archives are preserved.')
replacements={'RDR2VulkanFG.asi':args.asi.read_bytes()}
if args.interposer:replacements['RDR2VulkanFG/Streamline/sl.interposer.dll']=args.interposer.read_bytes()
if args.diagnostics:replacements['RDR2VulkanFG/RDR2VulkanFGDiagnostics.dll']=args.diagnostics.read_bytes()
if args.core:replacements['RDR2VulkanFG/Core/RTX40MFGCore.dll']=args.core.read_bytes()
project=Path(__file__).resolve().parents[1]
for name in ('README.md','README.txt','CHANGELOG.md','SOURCE-NOTES.txt','VALIDATION.txt','LICENSE.txt','ACKNOWLEDGEMENTS.txt'):
    replacements[name]=(project/name).read_bytes()
with zipfile.ZipFile(args.template) as original:
    names=original.namelist()
    required={'RDR2VulkanFG.asi','RDR2VulkanFG.ini','RDR2VulkanFG/Streamline/sl.interposer.dll',
              'RDR2VulkanFG/Core/RTX40MFGCore.dll','RDR2VulkanFG/XeSS/libxess_fg.dll'}
    if not required.issubset(names) or len(names)!=len(set(names)):
        parser.error('Expected a complete compatible runtime template with unique entries.')
    if any('\\' in name or name.startswith('/') or '..' in Path(name).parts for name in names):
        parser.error('Runtime template contains an unsafe archive path.')
    ini=original.read('RDR2VulkanFG.ini').decode('utf-8-sig')
    for value in ('DebugLogging=0','Diagnostics=0','IntervalLogging=0','EnableExperimentalRTX20=0'):
        if value not in ini:parser.error('Runtime template has unexpected default settings.')
    for line in ini.splitlines():
        if not line.lstrip().startswith(';') and line.strip().startswith(('Developer','DevTest','Simulate')):
            parser.error('Runtime template contains developer or simulation keys.')
    if any(n.endswith(('.exe','.pdb','.log','.jsonl')) for n in names):
        parser.error('Runtime template contains development artifacts.')
    ini=re.sub(r'(?m)^; RDR2 Vulkan Frame Generation[^\r\n]*',
               '; RDR2 Vulkan Frame Generation 1.0',ini)
    replacements['RDR2VulkanFG.ini']=ini.encode('utf-8')
    def obsolete(name):
        return '/' not in name and (name=='SHA256SUMS.txt' or re.fullmatch(r'(README|CHANGELOG)-[0-9][A-Za-z0-9.\-]*\.txt',name))
    with zipfile.ZipFile(args.output,'x',zipfile.ZIP_DEFLATED) as result:
        for name in names:
            if not obsolete(name):result.writestr(name,replacements.get(name,original.read(name)))
        for name,content in replacements.items():
            if name not in names:result.writestr(name,content)
with zipfile.ZipFile(args.output) as result:
    assert result.testzip() is None
    for name,content in replacements.items():assert result.read(name)==content
print(args.output)
print('SHA256:',hashlib.sha256(args.output.read_bytes()).hexdigest())
