# Persistent Mutes

[![GitHub release](https://img.shields.io/github/release/allejo/persistentMute.svg)](https://github.com/allejo/persistentMute/releases/latest)
![Minimum BZFlag Version](https://img.shields.io/badge/BZFlag-v2.4.12+-blue.svg)
[![License](https://img.shields.io/github/license/allejo/persistentMute.svg)](https://github.com/allejo/persistentMute/blob/master/LICENSE.md)

A BZFlag plug-in that will save a copy of mutes in-memory for 5 hours. Mutes are saved on a per IP basis and will prevent players from rejoining to bypass the mute.

## Requirements

- BZFlag 2.4.12

## Usage

**Loading the plug-in**

Load the plug-in without any command line arguments or configuration.

```
-loadplugin persistentMute
```

**Custom Slash Commands**

| Command | Permission | Description |
| ------- | ---------- | ----------- |
| `/mutelist` | mute | Overrides the built-in command and displays currently saved mutes and their time remaining |
| `/unmute` | mute | Overrides the built-in command and allows for removal of saved mutes even when the player is no longer on the server |

## License

[MIT](https://github.com/allejo/persistentMute/blob/master/LICENSE.md)
