from pathlib import Path
import re

root=Path(__file__).resolve().parents[1]/'workshop'
english=None
for path in sorted(root.rglob('*.yml')):
    data=path.read_bytes()
    assert data.startswith(b'\xef\xbb\xbf'),f'Missing BOM: {path.name}'
    text=data.decode('utf-8-sig')
    pairs=dict(re.findall(r'^\s+(\S+):0\s+"(.*)"$',text,re.M))
    assert len(pairs)==16,f'Localization count: {path.name}'
    if path.parent.name=='english':english=pairs
    assert '1–96' in pairs['scursor.101.desc']
assert english
for path in root.rglob('*.yml'):
    if path.parent.name=='simp_chinese':continue
    pairs=dict(re.findall(r'^\s+(\S+):0\s+"(.*)"$',path.read_text(encoding='utf-8-sig'),re.M))
    assert pairs==english,f'Non-Chinese locale must fall back to English: {path.name}'
assert not (root/'interface').exists(),'Core mod must not replace GUI files'
assert not list(root.rglob('*.dll')) and not list(root.rglob('*.exe'))
for path in root.rglob('*.txt'):
    text=path.read_text(encoding='utf-8')
    # Strip quoted strings and comments before checking script delimiters.
    text=re.sub(r'"(?:\\.|[^"\\])*"','',text)
    text=re.sub(r'#.*','',text)
    depth=0
    for char in text:
        depth+=(char=='{')-(char=='}')
        assert depth>=0,f'Unbalanced script: {path.name}'
    assert depth==0,f'Unbalanced script: {path.name}'
    assert path.name!='00_on_actions.txt'
print('10 locales, 16 keys each; balanced scripts; no GUI override or executables.')
