# CSB3 Bytecode Format

## Types

### Basic Types

All basic types are stored little-endian (LSB first).

- `int8`: 8-bit signed integer, two's complement
- `uint8`: 8-bit unsigned integer
- `int16`: 16-bit signed integer, two's complement
- `uint16`: 16-bit unsigned integer
- `int32`: 32-bit signed integer, two's complement
- `uint32`: 32-bit unsigned integer
- `int64`: 64-bit signed integer, two's complement
- `uint64`: 64-bit unsigned integer
- `float32`: 32-bit IEEE 754 single precision floating-point number
- `float64`: 64-bit IEEE 754 double precision floating-point number

**Note**: Any type postfixed with `*` is an offset in the file to which one or more of the type is stored contiguously, stored as a `uint64`.

### `byte`

A `byte` is a byte of data that lacks a specific type, with its purpose being to store raw data. It is stored as a `uint8`.

### `bool`

A `bool` is a boolean value, stored as a `uint8` where `0` is `false` and `1` is `true`.

### `string`

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `0x00` | `data` | `byte[]` | A UTF-8 encoded string, terminated with a `0x00` byte |

### `Script`

Offset | Name | Type | Description |
-------|------|------|-------------|
`0x00` | `offset` | `uint64` | Offset of the script |

### `Costume`

| Offset | Name | Type | Description | 
|--------|------|------|-------------|
| `0x00` | `name` | [`string *`](#string) | The name of the costume |
| `0x08` | `format` | [`string *`](#string) | The format of the costume |
| `0x10` | `bitmapResolution` | `uint32` | Bitmap resolution |
| `0x14` | `reserved` | `uint32` | Reserved, always `0` |
| `0x18` | `rotationCenterX` | `float64` | Rotation center X |
| `0x20` | `rotationCenterY` | `float64` | Rotation center Y |
| `0x28` | `dataSize` | `uint64` | The size of the costume data |
| `0x30` | `data` | [`byte *`](#byte) | The costume data |

### `Sound`

| Offset | Name | Type | Description | 
|--------|------|------|-------------|
| `0x00` | `name` | [`string *`](#string) | The name of the sound
| `0x08` | `format` | [`string *`](#string) | The format of the sound
| `0x10` | `rate` | `float64` | The rate of the sound
| `0x18` | `sampleCount` | `uint64` | Number of samples in the sound
| `0x20` | `dataSize` | `uint64` | The size of the sound data
| `0x28` | `data` | [`byte *`](#byte) | The sound data

### `Sprite`

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `0x00` | `name` | [`string *`](#string) | The sprite's name |
| `0x08` | `x` | `float64` | X position |
| `0x10` | `y` | `float64` | Y position |
| `0x18` | `direction` | `float64` | Direction |
| `0x20` | `size` | `float64` | Size |
| `0x28` | `currentCostume` | `int64` | Current costume |
| `0x30` | `layer` | `int64` | Render layer |
| `0x38` | `visible` | [`bool`](#bool) | Visibility |
| `0x40` | `isStage` | [`bool`](#bool) | Whether this sprite is the Stage |
| `0x48` | `draggable` | [`bool`](#bool) | Whether this sprite is draggable |
| `0x50` | `rotationStyle` | `uint8` | The rotation style |
| `0x58` | `numScripts` | `uint64` | Number of scripts |
| `0x60` | `initializer` | [`Script`](#script) | The initializer script |
| `0x68` | `scripts` | [`Script *`](#script) | Array of `numScripts` scripts |
| `0x70` | `numCostumes` | `uint64` | Number of costumes |
| `0x78` | `costumes` | [`Costume *`](#costume) | Array of `numCostumes` costumes |
| `0x80` | `numSounds` | `uint64` | Number of sounds |
| `0x88` | `sounds` | [`Sound *`](#sound) | Array of `numSounds` sounds |
| `0x90` | `numVariables` | `uint64` | Number of variables |
| `0x98` | `variables` | [`Value *`](#value) | Array of `numVariables` values |

### `Value`

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `0x00` | `uint16` | `uint16` | The type of the value |
| `0x02` | `flags` | `uint16` | Flags |
| `0x04` | `padding` | `uint32` | Padding, zeroed |
| `0x08` | `data` | `byte[8]` | The value data, interpreted based on the type |

### `VarId`

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `0x00` | `id` | `uint8[3]` | The variable ID, as a 3-byte LE unsigned integer |

## Structure

The bytecode is laid out as a header followed by a series of segments, each with a specific purpose. The header contains offsets to each segment, which are stored in the order they are listed below.

### Header

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `0x00` | `magic` | `uint32` | `0x33425343`, "CSB3" |
| `0x04` | `version` | `uint32` | Program version, always `1` |
| `0x08` | `text` | `uint32` | Offset of the [`.text`](#text) segment |
| `0x0c` | `text_size` | `uint32` | Size of the [`.text`](#text) segment |
| `0x10` | `stable` | `uint32` | Offset of the [`.stable`](#stable) segment |
| `0x14` | `stable_size` | `uint32` | Size of the [`.stable`](#stable) segment |
| `0x18` | `data` | `uint32` | Offset of the [`.data`](#data) segment |
| `0x1c` | `data_size` | `uint32` | Size of the [`.data`](#data) segment |
| `0x20` | `rdata` | `uint32` | Offset of the [`.rdata`](#rdata) segment |
| `0x24` | `rdata_size` | `uint32` | Size of the [`.rdata`](#rdata) segment |
| `0x28` | `debug` | `uint32` | Offset of the [`.debug`](#debug) segment |
| `0x2c` | `debug_size` | `uint32` | Size of the [`.debug`](#debug) segment |

### `.text`

Executable code segment.

| Offset | Name | Type | Description |
|--------|------|------|------------- |
| `0x00` | `bytecode` | `byte[]` | The compiled script bytecode |

### `.stable`

Sprite table segment.

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `0x00` | `count` | `uint64` | Number of sprites in the table |
| `0x08` | `sprites` | [`Sprite[]`](#sprite) | The sprites, length `count` |

### `.data`

Read-write data segment.

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `0x00` | `data` | `byte[]` | The data |

### `.rdata`

Read-only data segment.

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `0x00` | `data` | `byte[]` | The data |

### `.debug`

Debug information segment.

**Note**: No information is currently stored here.

## Bytecode

The bytecode is a sequence of varying-length instructions, each with an opcode and a set of operands. The bytecode is stored in the [`.text`](#text) segment.

### Instruction Format

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `0x00` | `opcode` | `uint8` | The opcode |
| `0x01` | `operands` | `byte[]` | The operands |

Instructions may also require stack-based operands, which are not stored in the bytecode. All instructions pop their operands from the stack and not push them back.
