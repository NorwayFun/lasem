Release 0.6.0
=============

  * parser: fix libxml2 deprecation warning (Emmanuel)

Release 0.5.2
=============

  * viewer: new simple mathml and svg viewer (mjakeman)
  * itex2mml: update to 1.6.1 (Emmanuel)
  * build: switch to meson (Emmanuel)
  * build: deprecation warning fixes (Emmanuel)
  * documentation: port API doc to gi-docgen (Emmanuel)
  * ci: linux pipeline, valgrind checks, publication of aravis test suite rendering (Emmanuel)

Release 0.5.1
=============

  * svg: implement overflow, viewBox and preserveAspectRatio for <symbol> (Emmanuel)
  * svg: fix a crash in blur filter in case of a null input surface (Emmanuel
  * svg: <feColorMatrix> support
  * svg: <feImage> support
  * svg: <feMorphology> support
  * svg: <feConvolveMatrix> support
  * svg: <feTurbulence> support
  * svg: <feDisplacementMap> support
  * svg: use Mario Klingeman's stackblur algorithm for <feBlur>
  * mathml: negative spacing constant support (Garen)
  * mathml: minlabelspacing support for <mlabeledtr> (Garen)
  * introspection: better python support (Emmanuel)
  * cairo: avoid integer oveflow (RyuzakiKK)
  * build: OSX compilation fix (Garen)
  * translation updates (Мирослав, Aurimas, Balázs, Piotr, Tom, Anders, Daniel, Andika, Marek, Pedro, Duarte, Cédric, Samir, Serdar, Ask, Milo, Rūdolfs, Nathan, Matej, Fabio, Aron, Mario, Rafael, Andika)

Release 0.5.0
=============

  * menclose mathml element support (emmanuel)
  * build improvements (ray, emmanuel, dominique)
  * man page for lasem-render (ray)
  * svg: move element registering by id to SvgDocument
  * dom: store owner_document as node member
  * itex: export new functions for itex to matml conversion
  * translation updates (Piotr, Daniel, Matej, Andika, Mario, Rafael, Fran, Balázs, Мирослав, Marek, Dimitris, Aurimas, Muhammet, Kjartan, Anders)

Release 0.4.0
=============

  * add a zoom option to lasem-render
  * support out-of-source build (Ray Dassen)
  * explicitely link against standard math library (Ray Dassen)
  * GIR scanner warning fixes
  * svg: add support for viewport-fill and viewport-opacity attributes
  * svg: add support for comp-op attributes
  * svg: better fix for blur offset from a patch found in librsvg bugzilla (Eduard Braun)

Release 0.3.4
=============

  * display result of comparison to reference file in test suite
  * svg: use cairo API for filter implementation when possible
  * svg: fix blur misposition
  * svg: support for filter primitive subregion
  * svg: mplementation of <feTile>
  * svg: implementation of BackgroundImage and BackgroundAlpha
  * svg: implementation of <switch>
  * svg: reimplement group opacity optimisation
  * svg: implementation of get_extents for <polygon>, <polyline>, <ellipse>, <line> and <text>
  * svg: add support for HSL color syntax
  * svg: fix parsing of stroke-dasharray when value are separated by spaces

Release 0.3.3
=============

  * fix parallel compilation
  * svg: fix a crash in case of transform attribute on gradient elements
  * svg: handle nodes with name prefixed with 'svg:'
  * svg: lame vertical text support
  * svg: fix font size when expressed as percentage
  * svg: implement overflow attribute for markers
  * svg: fix Marker auto orientation
  * svg: fix clipPath transform
  * svg: add preliminary support for <feGaussianBlur>, <feBlend>, <feComposite>,
  <feOffset>, <feMerge>, <feMergeNode> and <feFlood>
  * svg: fix group opacity
  * svg: use transparent color when paint url is not found
  * mml: support for element prefixed by 'math:'
  * mml: fix update of itex element on style change

Release 0.3.2
=============

  * restrict introspection to the DOM API
  * mml: fix weight of operators
  * mml: support of semantics elements (used by libreoffice formulas)
  * mml: fix the form of script elements of underover and script
  * svg: garbage collection in property manager
  * svg: avoid cairo errors due to not invertible matrix

Release 0.3.1
=============

  * single header include
  * gtk-doc support
  * unit testing
  * only export and document the DOM API
  * binary are now parallel installable
  * fix crashers found using fuzzxml from Morten Welinder
  * improve debug output (À la gstreamer)
  * dom: pass a cairo context for each view render
  * dom: better DOM specification conformance
  * dom: serialization support
  * itex2mml: update to 1.4.5
  * mml: return equation baseline.
  * svg: fix gradient on stroke or fill when opacity < 1.0
  * svg: implement visibility attribute
  * svg: implement display attribute
  * svg: implement image::get_extents
  * svg: avoid infinite circular references

Release 0.3.0
=============

  * add gobject-instrospection
  * switch to LGPLv2+ licence
  * itex2mml: update to 1.3.22
  * svg: use pango_cairo_show_layout for text fill
  * svg: font-weight, font-style, font-stretch and text-anchor support
  * svg: <tspan> support
  * svg: fix position of outermost <svg>
  * svg: href support for <pattern>
  * svg: href support for <radialGradient>
  * svg: href support for <lienarGradient>
  * svg: allow commas in viewBox string
  * svg: fix ellipse rendering direction
  * svg: fix parsing of rgb() colors
  * svg: don't send invalid dash array to cairo

Release 0.2.0
=============

  * make lasem library parallel installable

Release 0.1.3
=============

  * itex2mml: update to the 1.3.15 release
  * mml: contribution of Jorn Baayen needed for his lasem python bindings
  http://github.com/jbaayen/pylasem
  * mml: migration of the attribute handling to lsm_attribute_manager
  svg: leak fixes
