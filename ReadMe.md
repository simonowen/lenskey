# LensKey

## Introduction

LensKey is a Lenslok™ decoder for Windows. It emulates the function of the
plastic lens, unscrambling an on-screen pattern to reveal a 2 character security
code.

The following Lenslok-protected titles are supported:

* ACE
* Art Studio
* Elite
* Graphic Adventure Creator
* Jewels of Darkness
* Mooncresta
* Price of Magik
* Tomahawk
* TT Racer

## Usage

1) Start LensKey and select a software title from the drop-down list. Each title
   uses a slightly different encoding method, so the correct one must be
   selected.

2) Click on the main area in the Lens Viewer window to enter selection mode.
   This allows a pattern region to be selected for decoding. The cursor changes
   to a cross-hair until a selection has been made.

3) On the emulator window, select the right-hand half of the Lenslok pattern.
   Drag out a selection rectangle with the left edge of the selection in the
   middle of the central line. The top of the box should be just above the
   character pattern, and the bottom of the box should be just below it.

4) Continue dragging to the right until the OK test pattern is visible in the
   viewer window. With most patterns, you'll need to extend the selection area
   slightly beyond the right edge of the pattern.

5) Finally, press return/space in the emulator to display the real pattern,
   which will be decoded in the viewer window. Enter the code in the Spectrum
   emulator and you're in!

## Troubleshooting

If you're using an emulator I'd recommended that you pause it while using
LensKey, otherwise the frequent emulator screen updates may obscure the
selection box, making it difficult to see.

Can't see any recognisable characters in the viewer window?

* Check the software title selection is correct.
* Ensure the left edge of the region selection is on the central line.
* Ensure you're dragging down+right, and not up+left.
* Re-select the region if the target window has been moved or resized.
* Ensure the emulator is not using a video overlay surface for its display.
* Try pausing the emulator and/or turning off 'scanline' effects.

## License

The LensKey source code is released under the [MIT
license](https://tldrlegal.com/license/mit-license).

## Links

* Read more about the Lenslok system at Richard Hewison's [Bird
  Sanctuary](https://web.archive.org/web/20190308042601/http://birdsanctuary.co.uk/lenslok/)
  site.
* LensKey even got a passing mention in an [NTK
  newsletter](http://www.ntk.net/2002/10/04/)

*Lenslok is a trademark of ASAP Developments Ltd.*

---

Simon Owen  
<https://github.com/simonowen/lenskey>
