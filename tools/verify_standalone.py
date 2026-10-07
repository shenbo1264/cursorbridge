"""Verify an EXE-only distribution and its embedded runtime without attaching to a user app."""
from pathlib import Path
import argparse, hashlib, json, os, shutil, subprocess, tempfile, time
from verify_app_icon import verify_app_icon

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir',type=Path,default=Path(__file__).resolve().parents[1]/'build')
    args=parser.parse_args();build=args.build_dir.resolve()
    manifest=json.loads((build/'embedded/runtime_manifest.json').read_text(encoding='utf-8'))
    with tempfile.TemporaryDirectory(prefix='CursorBridge 单文件 EXE ') as temporary:
        root=Path(temporary);launch=root/'Only EXE';launch.mkdir()
        exe=launch/'CursorBridge.exe';shutil.copyfile(build/'bin/StellarisCursor.exe',exe)
        verify_app_icon(exe)
        target=root/'Never run target.exe';shutil.copyfile(build/'bin/StellarisCursorTest.exe',target)
        data=root/'Data';log=data/'logs/controller.log'
        # Select our non-running test host explicitly: never auto-attach to a
        # real game during a packaging check. No settings window/input is used.
        child=subprocess.Popen([str(exe),'--background','--app-path',str(target),'--data-dir',str(data)],creationflags=subprocess.CREATE_NO_WINDOW)
        try:
            deadline=time.monotonic()+20
            while time.monotonic()<deadline:
                if child.poll() is not None:raise RuntimeError('Standalone exited before startup')
                text=log.read_text(encoding='utf-8') if log.exists() else ''
                if 'CursorBridge started with the opt-in Windows x64' in text:break
                time.sleep(.1)
            else:raise RuntimeError('Standalone did not complete startup; another companion may be running')
            assert 'Verified embedded runtime ready:' in text
            assert '已连接 PID' not in text,'Unexpected target attachment'
            cache=Path(os.environ['LOCALAPPDATA'])/'CursorBridge/runtime'/manifest['key']
            for item in manifest['files']:
                path=cache/item['path'];content=path.read_bytes()
                assert len(content)==item['size'] and hashlib.sha256(content).hexdigest()==item['sha256'],item['path']
            assert list(launch.iterdir())==[exe],'Distribution directory contains sidecar files'
            stop=subprocess.run([str(exe),'--stop'],creationflags=subprocess.CREATE_NO_WINDOW,timeout=25)
            assert stop.returncode==0 and child.wait(timeout=5)==0
            print(f'EXE-only Unicode/spaced path launch passed; {len(manifest["files"])} embedded files verified; exact-path stop passed.')
        finally:
            if child.poll() is None:
                child.terminate();child.wait(timeout=5)

if __name__=='__main__':main()
