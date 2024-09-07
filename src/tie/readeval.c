// bezier quadratic .22.points.0 22./3 history.-2.23.curves.1.points.0
// ( make a quadratic bezier with control points made up of:
//      - point 0 of object 22 of the current context
//      - one third of the object 22 of the global context
//      - point 0 of curve 1 of object 23 of the second to last history context
//   the .points and .curves are for easier access and aren't real objects, or
//   rather, they're generated on the fly and shouldn't need to be reevaluated
//   in dynamic expressions as they should always refer to the same object )
// bq.22.p.0,22./3,-2#23.c.1.p.0
// ( short form )
