# -*- mode: python -*-

block_cipher = None


a = Analysis(['IRmqtt-tool.py'],
             pathex=['D:\\ownCloud\\Study\\git\\IRmqtt\\IRmqtt_tool'],
             binaries=[],
             datas=[],
             hiddenimports=['os','sys','socket','json','time','threading','queue','requests'],
             hookspath=[],
             runtime_hooks=[],
             excludes=[],
             win_no_prefer_redirects=False,
             win_private_assemblies=False,
             cipher=block_cipher,
             noarchive=False)
pyz = PYZ(a.pure, a.zipped_data,
             cipher=block_cipher)
exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          [],
          name='IRmqtt-tool',
          debug=False,
          bootloader_ignore_signals=False,
          strip=False,
          upx=True,
          runtime_tmpdir=None,
          console=True )
