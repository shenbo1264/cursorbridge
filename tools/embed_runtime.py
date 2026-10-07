"""Embed the hook and independently regenerated original themes in the executable."""
from pathlib import Path
import argparse,hashlib,json,tempfile
from make_themes import generate

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--dll',type=Path,required=True);parser.add_argument('--themes',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();args.output.mkdir(parents=True,exist_ok=True)
    files=[('StellarisCursorHook.dll',args.dll.resolve())]
    with tempfile.TemporaryDirectory(prefix='cursorbridge-embed-original-') as temporary:
        generated=Path(temporary);generate(generated)
        for item in sorted(generated.rglob('*')):
            if not item.is_file():continue
            relative=item.relative_to(generated);source=(args.themes/relative).resolve()
            assert source.read_bytes()==item.read_bytes(),f'Non-original cursor: {relative}'
            files.append(('assets/themes/'+relative.as_posix(),source))
    assert len(files)==109
    digest=hashlib.sha256();rows=[];rc=['#include <windows.h>'];manifest=[]
    for index,(name,source) in enumerate(files,500):
        data=source.read_bytes();digest.update(name.encode('utf-8')+b'\0'+hashlib.sha256(data).digest())
        rc.append(f'{index} RCDATA "{source.as_posix()}"')
        rows.append('{L"'+name.replace('/', '\\\\')+'",'+str(index)+','+str(len(data))+'}')
        manifest.append({'path':name,'id':index,'size':len(data),'sha256':hashlib.sha256(data).hexdigest()})
    (args.output/'runtime.rc').write_text('\n'.join(rc)+'\n',encoding='utf-8')
    header='#pragma once\nstruct EmbeddedFile {const wchar_t* path;WORD id;DWORD size;};\n'
    header+='static const wchar_t* RuntimeKey=L"'+digest.hexdigest()[:24]+'";\n'
    header+='static const EmbeddedFile EmbeddedFiles[]={\n'+',\n'.join(rows)+'\n};\n'
    (args.output/'runtime_manifest.h').write_text(header,encoding='utf-8')
    (args.output/'runtime_manifest.json').write_text(json.dumps({'key':digest.hexdigest()[:24],'files':manifest},indent=2),encoding='utf-8')
if __name__=='__main__':main()
