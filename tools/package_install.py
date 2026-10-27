"""Build the clean installation ZIP from the pinned runtime files."""
from pathlib import Path
import argparse,configparser,hashlib,json,re,zipfile

project=Path(__file__).resolve().parents[1]
version=re.search(r'kVersion="([0-9]+\.[0-9]+(?:\.[0-9]+)?)"',
                  (project/'src/branding.h').read_text(encoding='utf-8')).group(1)
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--runtime-root',type=Path,required=True,help='Directory containing Core, Streamline and XeSS')
parser.add_argument('--loader',type=Path,required=True,help='Official Ultimate ASI Loader dinput8.dll')
parser.add_argument('--asi',type=Path,default=project/'RDR2VulkanFG.asi')
parser.add_argument('--diagnostics',type=Path,default=project/'RDR2VulkanFG/RDR2VulkanFGDiagnostics.dll')
parser.add_argument('--output',type=Path,required=True)
parser.add_argument('--manifest',type=Path,help='Optional external JSON verification report, not included in the ZIP')
args=parser.parse_args()
if args.output.exists():parser.error('Choose a new output path; existing archives are preserved.')
digest=lambda data:hashlib.sha256(data).hexdigest()
reference=json.loads((project/'runtime-manifest.json').read_text(encoding='utf-8'))
files={}
for relative,expected in reference['runtime_files'].items():
    if Path(relative).is_absolute() or '..' in Path(relative).parts or '\\' in relative or not relative.endswith('.dll'):
        parser.error('Unsafe runtime manifest path.')
    path=args.runtime_root/relative
    if not path.is_file():parser.error(f'Required runtime file is missing: {relative}')
    data=path.read_bytes()
    if digest(data)!=expected:parser.error(f'Runtime identity differs from the pinned family: {relative}')
    files['RDR2VulkanFG/'+relative]=data
loader=args.loader.read_bytes()
if digest(loader)!=reference['loader_sha256']:parser.error('The loader differs from the verified official build.')
files['dinput8.dll']=loader
files['RDR2VulkanFG.asi']=args.asi.read_bytes()
files['RDR2VulkanFG/RDR2VulkanFGDiagnostics.dll']=args.diagnostics.read_bytes()
profile=str(Path.home())
private_paths=[profile.encode('utf-8'),profile.replace('\\','/').encode('utf-8'),profile.encode('utf-16le')]
for name in ('RDR2VulkanFG.asi','RDR2VulkanFG/RDR2VulkanFGDiagnostics.dll'):
    data=files[name]
    if not data.startswith(b'MZ') or any(path in data for path in private_paths):
        parser.error(f'Invalid build or private path in {name}')
if b'MenuPreviewConfigure' in files['RDR2VulkanFG.asi']:parser.error('A preview fixture cannot be packaged as the ASI.')
ini=(project/'RDR2VulkanFG.ini').read_bytes()
config=configparser.ConfigParser(interpolation=None);config.read_string(ini.decode('utf-8-sig'))
for name in ('DebugLogging','Diagnostics','IntervalLogging','EnableExperimentalRTX20'):
    if config['fgvk'].get(name)!='0':parser.error(f'Unsafe release default: {name}')
if any(key.startswith(('developer','devtest','simulate')) for key in config['fgvk']):parser.error('Developer keys are not release settings.')
files['RDR2VulkanFG.ini']=ini
files['RDR2VulkanFG/Core/RTX20/dlssg_sm86.ini']=b'''[General]
Enabled=1
[FrameGeneration]
Engine=DLSS
MaxGeneratedFrames=5
[DLSSG]
Optimized=1
Preset=Auto
[Runtime]
Mode=Bundled
[SuperResolution]
Mode=Off
[Logging]
Level=0
Directory=dlssg_sm86\\logs
'''
# Documentation stays in the repository. Mandatory notices are grouped here.
for path in (project/'licenses').rglob('*'):
    if path.is_file():files['licenses/'+path.relative_to(project/'licenses').as_posix()]=path.read_bytes()
files['licenses/Project-LICENSE.txt']=(project/'LICENSE.txt').read_bytes()
files['licenses/Project-ACKNOWLEDGEMENTS.txt']=(project/'ACKNOWLEDGEMENTS.txt').read_bytes()
for relative in reference['runtime_notices']:
    if Path(relative).is_absolute() or '..' in Path(relative).parts or '\\' in relative:
        parser.error('Unsafe runtime notice path.')
    path=args.runtime_root/relative
    if not path.is_file():parser.error(f'Required third-party notice is missing: {relative}')
    files['licenses/Runtime/'+relative]=path.read_bytes()
# Keep a single mandatory notice file instead of scattering text files.
notice_files={name:data for name,data in files.items() if name.startswith('licenses/')}
groups={}
for name,data in sorted(notice_files.items()):
    groups.setdefault(digest(data),{'names':[],'data':data})['names'].append(name)
parts=[f'RDR2 Vulkan Frame Generation {version} - Required license and attribution notices\n\n'.encode('utf-8')]
for group in groups.values():
    parts.append(('='*72+'\nOriginal notice files: '+', '.join(group['names'])+'\n'+'='*72+'\n\n').encode('utf-8'))
    parts.append(group['data']);parts.append(b'\n\n')
combined=b''.join(parts)
assert all(data in combined for data in notice_files.values())
files={name:data for name,data in files.items() if not name.startswith('licenses/')}
files['licenses/NOTICES.txt']=combined
required_root={'dinput8.dll','RDR2VulkanFG.asi','RDR2VulkanFG.ini'}
assert {name for name in files if '/' not in name}==required_root
assert all(not name.startswith('/') and '..' not in Path(name).parts and '\\' not in name for name in files)
assert not any(name.lower().endswith(('.log','.pdb','.dmp','.csv','.bak','.exe')) for name in files)
assert all(not name.lower().endswith(('.txt','.md')) or name.startswith('licenses/') for name in files)
args.output.parent.mkdir(parents=True,exist_ok=True)
with zipfile.ZipFile(args.output,'x',zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
    for name,data in sorted(files.items()):archive.writestr(name,data)
with zipfile.ZipFile(args.output) as archive:
    assert archive.testzip() is None
    assert set(archive.namelist())==set(files)
    for name,data in files.items():assert digest(archive.read(name))==digest(data)
report={'version':version,'package_type':'complete mod installation; the game and system prerequisites are external',
        'root_files':sorted(required_root),'file_count':len(files),'zip_bytes':args.output.stat().st_size,
        'zip_sha256':digest(args.output.read_bytes()),'files':{name:digest(data) for name,data in sorted(files.items())},
        'documentation_in_zip':False,'legal_notices_in_licenses_directory':True,'personal_configuration_included':False,
        'runtime_identity_verification':'PASS','archive_integrity':'PASS'}
if args.manifest:args.manifest.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps({key:value for key,value in report.items() if key!='files'},indent=2))
