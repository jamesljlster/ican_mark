# ICANMark

A utility for rotated bounding box annotating, currently.  
Still under heavy developments.

### Annotation File Format

example:

``` yaml
# File name: sample.mark
# File format: yaml
- label: 0
  degree: 90 # (degree)
  x: 0 # (pixel, center)
  y: 0 # (pixel, center)
  w: 60 # (pixel)
  h: 30 # (pixel)
  segment:
    - [x: 0, y: 0] # (pixel)
    - [x: 1, y: 1]
    - [x: 1, y: 0]
- label: 1
  # ...
```
