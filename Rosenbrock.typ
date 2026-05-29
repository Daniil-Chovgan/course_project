#import "p"

#lq.diagram(
   width: 4cm, height: 4cm,
   lq.contour(
     lq.linspace(-5, 5, num: 12),
     lq.linspace(-5, 5, num: 12),
     (x, y) => x * y,
     map: color.map.icefire,
   )
 )