# Untwine

Untwine is software from [Hobu, Inc.](https://hobu.co) for creating [Entwine Point Tile](https://entwine.io/entwine-point-tile.html)  (EPT)
web services from [PDAL](https://pdal.io)-readable point cloud data sources. It
provides an alternative processing approach than the [Entwine](https://entwine.io)
software, but the output is expected to be compatible EPT.


License
-------

Untwine is licensed under the GPLv3. Commercial licensing is possible by contacting Hobu, Inc. for pricing.

Using Untwine
-------------

```
untwine [options]
```

Example:
--------

```
untwine --files=some_directory --output_dir=output_directory
```

Options
-------

- files

  Input files or directories containing input files. [Required]

- output_dir

  Output directory. [Required]

- dims

  List of dimensions to load. X, Y and Z are always loaded. Limiting the dimensions can
  speed runtime and reduce temporary disk use.

- temp_dir

  Directory in which to place tiled output. If not provided, temporary files are placed
  in 'output_dir'/temp.

- cube

  Create a voxel structure where each voxel is a cube. If false, the voxel structure is
  a rectangular solid that encloses the points. [Default: true]

- level

  Level to use when initially tiling points.  If not provided, an initial level is
  determined from the data. [Default: none].

- file_limit

  Only read 'file_limit' input files even if more exist in the 'files' list. Used primarily
  for debugging. [Default: no limit]
