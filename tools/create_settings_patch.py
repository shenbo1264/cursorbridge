"""Generate a LOCAL settings-page button from an installed UI. Do not upload the generated GUI."""
from pathlib import Path
import argparse
import re

BUTTON = '''
        # CURSORBRIDGE_LOCAL_ENTRY_BEGIN
        effectbuttonType = {
            name = "scursor_native_settings_entry"
            position = { x = -330 y = 7 }
            size = { x = 320 y = 40 }
            orientation = upper_right
            quadTextureSprite = "GFX_tiling_button_standard"
            font = "FONT_NAME"
            buttonText = "scursor.native_settings"
            tooltipText = "scursor.native_settings_tip"
            effect = scursor_native_settings_button
            clicksound = "click"
        }
        # CURSORBRIDGE_LOCAL_ENTRY_END
'''
EFFECT = '''scursor_native_settings_button = {
    potential = { always = yes }
    allow = { exists = from is_multiplayer = no }
    effect = {
        hidden_effect = {
            from = {
                if = {
                    limit = { is_variable_set = scursor_ui_nonce }
                    change_variable = { which = scursor_ui_nonce value = 1 }
                }
                else = { set_variable = { which = scursor_ui_nonce value = 1 } }
                log = "SCURSOR_V1_OPEN_SETTINGS_N_[This.scursor_ui_nonce]"
            }
        }
    }
}
'''

def generate(source: Path, output: Path, font: str):
    source=source.resolve();output=output.resolve()
    if source.is_relative_to(output) or output.is_relative_to(source.parent):
        raise ValueError('Output must be separate from the source UI directory.')
    if output.exists():
        raise ValueError('Choose a new output directory; existing patches are not overwritten.')
    if not re.fullmatch(r'[A-Za-z0-9_]+',font):
        raise ValueError('Invalid font identifier.')
    original=source.read_text(encoding='utf-8-sig')
    match=re.search(r'name\s*=\s*"settings_view"',original)
    if not match or 'scursor_native_settings_entry' in original:
        raise ValueError('Unrecognized or already patched settings page.')
    button=BUTTON.replace('FONT_NAME',font)
    patched=original[:match.end()]+button+original[match.end():]
    assert patched.replace(button,'',1)==original
    (output/'interface').mkdir(parents=True)
    (output/'common/button_effects').mkdir(parents=True)
    (output/'interface/settings_view.gui').write_text(patched,encoding='utf-8')
    (output/'common/button_effects/cursorbridge_button.txt').write_text(EFFECT,encoding='utf-8')
    (output/'descriptor.mod').write_text('name="CursorBridge - Local Settings Button"\nversion="0.6.0"\nsupported_version="4.5.*"\ndependencies={ "CursorBridge - Stellaris Cursor Size" }\n',encoding='utf-8')
    print('Created a local patch. Load it AFTER your UI mods and the core CursorBridge mod. Regenerate after UI updates. Do not redistribute the generated GUI.')

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--gui-source',required=True,type=Path)
    parser.add_argument('--output',required=True,type=Path)
    parser.add_argument('--font',default='font_text_20',help='UOD: font_text_20; vanilla: cg_16b (separate validation required)')
    args=parser.parse_args()
    generate(args.gui_source,args.output,args.font)
