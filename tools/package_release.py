"""Build public ZIPs from an explicit original-file allowlist; omit private development data."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess
import zipfile
import tempfile
from make_themes import generate

ROOT=Path(__file__).resolve().parents[1]
VERSION='0.8.0'

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir',type=Path,default=ROOT/'build')
    args=parser.parse_args()
    build=args.build_dir.resolve()
    out=ROOT/'dist';out.mkdir(exist_ok=True)
    subprocess.run(['python',str(ROOT/'tools/validate_workshop.py')],check=True)
    documents=['README.md','README.zh-CN.md','LICENSE','SECURITY.md','docs/ARCHITECTURE.md','docs/ROADMAP.md','docs/VALIDATION.md','docs/YOLOMOUSE_COMPARISON.md','docs/UI_DESIGN.md','docs/WORKSHOP_UPLOAD.md','docs/WORKSHOP_DESCRIPTION.zh.bbcode','docs/WORKSHOP_DESCRIPTION.en.bbcode']
    images=[p.relative_to(ROOT).as_posix() for p in sorted(p for p in (ROOT/'docs/images').iterdir() if p.suffix in {'.png','.jpg'})]
    launchers=['Open Settings.cmd','Start Companion.cmd','Launch Stellaris.cmd','Stop Companion.cmd']
    toolkit=['tools/create_settings_patch.py']
    binaries=['StellarisCursor.exe','StellarisCursorHook.dll']
    entries={name:(ROOT/name).read_bytes() for name in documents+images+launchers+toolkit}
    for name in binaries:
        entries['bin/'+name]=(build/'bin'/name).read_bytes()
    # Only independently regenerated original artwork can enter the archive.
    # This makes an accidental copied game cursor fail packaging.
    own_themes=set()
    with tempfile.TemporaryDirectory(prefix='cursorbridge-original-themes-') as fixture:
        generated=Path(fixture);generate(generated)
        for asset in sorted(generated.rglob('*')):
            if not asset.is_file():continue
            relative=asset.relative_to(generated)
            data=(build/'bin/assets/themes'/relative).read_bytes()
            assert data==asset.read_bytes(),f'Non-original theme asset: {relative}'
            name='bin/assets/themes/'+relative.as_posix();entries[name]=data;own_themes.add(name)
    assert len(own_themes)==108
    mod={p.relative_to(ROOT).as_posix():p.read_bytes() for p in sorted((ROOT/'workshop').rglob('*')) if p.is_file()}
    assert len(mod)==15, f'Unexpected Workshop file count: {len(mod)}'
    entries.update(mod)
    for name,data in entries.items():
        assert not re.search(r'(?:^|/)(?:logs|local|userdata|research|staging)/',name),name
        assert not name.endswith(('.pdb','.obj','.lib')),name
        assert not name.endswith(('.cur','.ani')) or name in own_themes,name
        assert not name.endswith('settings_view.gui'),name
        for marker in (b'gho_',b'ghp_'):
            assert marker not in data,(name,marker)
        for private in (Path.home(),ROOT,ROOT.parent):
            for spelling in (str(private),private.as_posix()):
                for encoding in ('utf-8','utf-16le'):
                    assert spelling.encode(encoding) not in data,(name,'private build path')
    manifest={'version':VERSION,'platform':'Windows x64','game_adapter':'Stellaris 4.5.1','companion_required':True,'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'files':{name:hashlib.sha256(data).hexdigest() for name,data in sorted(entries.items())}}
    entries['manifest.json']=json.dumps(manifest,ensure_ascii=False,indent=2).encode('utf-8')
    packages={
        'CursorBridge-Stellaris-windows-x64.zip':entries,
        'CursorBridge-Stellaris-workshop-upload.zip':{name.removeprefix('workshop/'):data for name,data in mod.items()},
    }
    checksums=[]
    for filename,content in packages.items():
        path=out/filename
        with zipfile.ZipFile(path,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
            for name,data in sorted(content.items()):
                item=zipfile.ZipInfo(name,(2026,10,6,0,0,0));item.compress_type=zipfile.ZIP_DEFLATED
                archive.writestr(item,data)
        with zipfile.ZipFile(path) as archive:
            assert archive.testzip() is None
            assert set(archive.namelist())==set(content)
        digest=hashlib.sha256(path.read_bytes()).hexdigest()
        checksums.append(f'{digest}  {filename}\n')
        print(f'{filename}: {path.stat().st_size} bytes; {len(content)} files; SHA256 {digest}')
    (out/'SHA256SUMS.txt').write_text(''.join(checksums),encoding='ascii',newline='\n')

if __name__=='__main__':
    main()
