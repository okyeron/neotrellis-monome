# neotrellis_monome_teensy

Fork of [okeyron's neotrellis_monome_teensy firmware](https://github.com/okyeron/neotrellis-monome).

It seems, at least on Windows and with [my pyserialoscd implementation](https://github.com/nexxyz/pyserialoscd), the /grid/map/level (and possibly row & col) OSC command (when tested with monome home's vari-bright map-test) does not behave at expected with okeyron's original firmware. Please use my modified firmware provided instead. It also adds variable intensity for mono-bright applications using the /grid/intensity OSC command.
