"""Prepare an isolated local test profile, without editing a normal playset or starting a game."""
from pathlib import Path
import argparse
import json
import re

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',required=True,type=Path)
    parser.add_argument('--settings-from',required=True,type=Path)
    parser.add_argument('--mod',action='append',type=Path,default=[])
    parser.add_argument('--language',default='l_simp_chinese')
    args=parser.parse_args()
    if not re.fullmatch(r'l_[a-z_]+',args.language):
        raise ValueError('Invalid language token')
    output=args.output.resolve()
    if output.exists():
        raise ValueError('Choose a new directory; existing profiles are not overwritten.')
    text=args.settings_from.read_text(encoding='utf-8-sig')
    for key,value in [('fullScreen','no'),('borderless','no'),('gui_scale','1.000000'),('tutorial','0'),('language',f'"{args.language}"')]:
        text=re.sub(rf'(?m)^(\s*{key}\s*=).*$',rf'\g<1>{value}',text)
    text=re.sub(r'(?m)^(\s*x\s*=)\d+',r'\g<1>1920',text,count=1)
    text=re.sub(r'(?m)^(\s*y\s*=)\d+',r'\g<1>1200',text,count=1)
    # A minimal fresh-game configuration must include a valid galaxy template.
    text=re.sub(r'galaxy_config_sp\s*=\s*\{[^{}]*\}', 'galaxy_config_sp={ template="tiny" difficulty=captain }',text)
    (output/'mod').mkdir(parents=True)
    (output/'logs').mkdir()
    (output/'settings.txt').write_text(text,encoding='utf-8')
    pdx=args.settings_from.parent/'pdx_settings.txt'
    if pdx.is_file():
        preferences=pdx.read_text(encoding='utf-8-sig')
        preferences=re.sub(r'value="l_[a-z_]+"',f'value="{args.language}"',preferences)
        preferences=preferences.replace('"fullscreen"','"windowed"').replace('"borderless_fullscreen"','"windowed"')
        (output/'pdx_settings.txt').write_text(preferences,encoding='utf-8')
    mods=[]
    for number,directory in enumerate(args.mod):
        directory=directory.resolve()
        descriptor=(directory/'descriptor.mod').read_text(encoding='utf-8-sig')
        descriptor=re.sub(r'(?m)^\s*path\s*=.*\n?','',descriptor)
        descriptor+=f'\npath="{directory.as_posix()}"\n'
        name=f'mod/cursorbridge_smoke_{number}.mod'
        (output/name).write_text(descriptor,encoding='utf-8')
        mods.append(name)
    (output/'dlc_load.json').write_text(json.dumps({'disabled_dlcs':[],'enabled_mods':mods}),encoding='utf-8')
    print(f'Created an isolated profile with {len(mods)} mods. No normal settings were changed.')

if __name__=='__main__':
    main()
