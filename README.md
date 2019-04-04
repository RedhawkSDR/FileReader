# REDHAWK Basic Components rh.FileReader

## Description

Contains the source and build script for the REDHAWK Basic Components
rh.FileReader. The rh.FileReader component is responsible for reading data from
a file on the SCA or local file system, and streaming the data out a BULKIO
output port.

## Branches and Tags

All REDHAWK core assets use the same branching and tagging policy. Upon release,
the `master` branch is rebased with the specific commit released, and that
commit is tagged with the asset version number. For example, the commit released
as version 1.0.0 is tagged with `1.0.0`.

Development branches (i.e. `develop` or `develop-X.X`, where *X.X* is an asset
version number) contain the latest unreleased development code for the specified
version. If no version is specified (i.e. `develop`), the branch contains the
latest unreleased development code for the latest released version.

## REDHAWK Version Compatibility

| Asset Version | Minimum REDHAWK Version Required |
| ------------- | -------------------------------- |
| 5.x           | 2.0                              |
| 4.x           | 2.0                              |
| 3.x           | 1.10                             |
| 2.x           | 1.8                              |

## Installation Instructions
This asset requires the rh.blueFileLib and rh.RedhawkDevUtils shared libraries.
These shared libraries must be installed in order to build and run this asset.
To build from source, run the `build.sh` script found at the top level
directory. To install to $SDRROOT, run `build.sh install`. Note: root privileges
(`sudo`) may be required to install.

## Asset Use

To use rh.FileReader, configure the `source_uri` property with the path to the
file (or directory of files) to be read. Configure the `file_format` property
with the value appropriate for the input file(s). For files that do not contain
a header with sample rate and/or frequency information, configure the
`sample_rate` property with the sample rate and `center_frequency` with the
center frequency of the data. Connect an output port of the appropriate type to
the intended consumer of the data stream. To begin playback, configure the
`playback_state` property to `PLAY` and ensure the component has been started.

The BLUEFILE  and WAV options for the `file_format` property rely on the file
header for the full description of the data to be read. Each other option fully
describes the data to be read, and most options have up to four components:

1. Mode: SCALAR (Real) or COMPLEX
2. Atom size: 8 (Char/Octet), 16 (Short), 32 (Long/Float), or 64 (Double)
3. Data type: Unsigned integer, Signed integer, Floating-point
4. Byte Order: Big or Little Endian

All possible `file_format` property values are listed in the table below.

| Label | Value | Mode | Atom Size | Data Type | Byte Order |
| ----- | ----- | ---- | --------- | --------- | ---------- |
| BLUE/PLATINUM FILE | BLUEFILE | Header defined | Header defined | Header defined | Header defined |
| WAV | WAV | Scalar | Header defined | Header defined | Little Endian |
| XML | XML | Scalar | 8 | Char (Signed Integer) | N/A |
| SCALAR OCTET (8o) | OCTET | Scalar | 8 | Unsigned Integer | N/A |
| SCALAR CHAR (8t) | CHAR | Scalar | 8 | Signed Integer | N/A |
| SCALAR USHORT Little Endian (16or) | USHORT_LITTLE_ENDIAN | Scalar | 16 | Unsigned Integer | Little Endian |
| SCALAR USHORT Big Endian (16o) | USHORT_BIG_ENDIAN | Scalar | 16 | Unsigned Integer | Big Endian |
| SCALAR SHORT Little Endian(16tr) | SHORT_LITTLE_ENDIAN | Scalar | 16 | Signed Integer | Little Endian |
| SCALAR SHORT Big Endian (16t) | SHORT_BIG_ENDIAN | Scalar | 16 | Signed Integer | Big Endian |
| SCALAR ULONG Little Endian(32or) | ULONG_LITTLE_ENDIAN | Scalar | 32 | Unsigned Integer | Little Endian |
| SCALAR ULONG Big Endian (32o) | ULONG_BIG_ENDIAN | Scalar | 32 | Unsigned Integer | Big Endian |
| SCALAR LONG Little Endian (32tr) | LONG_LITTLE_ENDIAN | Scalar | 32 | Signed Integer | Little Endian |
| SCALAR LONG Big Endian (32t) | LONG_BIG_ENDIAN | Scalar | 32 | Signed Integer | Big Endian |
| SCALAR FLOAT Little Endian(32fr) | FLOAT_LITTLE_ENDIAN | Scalar | 32 | Floating-point | Little Endian |
| SCALAR FLOAT Big Endian (32f) | FLOAT_BIG_ENDIAN | Scalar | 32 | Floating-point | Big Endian |
| SCALAR DOUBLE Little Endian (64fr) | DOUBLE_LITTLE_ENDIAN | Scalar | 64 | Floating-point | Little Endian |
| SCALAR DOUBLE Big Endian (64f) | DOUBLE_BIG_ENDIAN | Scalar | 64 | Floating-point | Big Endian |
| COMPLEX OCTET (8o) | COMPLEX_OCTET | Complex | 8 | Unsigned Integer | N/A |
| COMPLEX CHAR (8t) | COMPLEX_CHAR | Complex | 8 | Signed Integer | N/A |
| COMPLEX USHORT Little Endian (16or) | COMPLEX_USHORT_LITTLE_ENDIAN | Complex | 16 | Unsigned Integer | Little Endian |
| COMPLEX USHORT Big Endian (16o) | COMPLEX_USHORT_BIG_ENDIAN | Complex | 16 | Unsigned Integer | Big Endian |
| COMPLEX SHORT Little Endian(16tr) | COMPLEX_SHORT_LITTLE_ENDIAN | Complex | 16 | Signed Integer | Little Endian |
| COMPLEX SHORT Big Endian (16t) | COMPLEX_SHORT_BIG_ENDIAN | Complex | 16 | Signed Integer | Big Endian |
| COMPLEX ULONG Little Endian(32or) | COMPLEX_ULONG_LITTLE_ENDIAN | Complex | 32 | Unsigned Integer | Little Endian |
| COMPLEX ULONG Big Endian (32o) | COMPLEX_ULONG_BIG_ENDIAN | Complex | 32 | Unsigned Integer | Big Endian |
| COMPLEX LONG Little Endian (32tr) | COMPLEX_LONG_LITTLE_ENDIAN | Complex | 32 | Signed Integer | Little Endian |
| COMPLEX LONG Big Endian (32t) | COMPLEX_LONG_BIG_ENDIAN | Complex | 32 | Signed Integer | Big Endian |
| COMPLEX FLOAT Little Endian(32fr) | COMPLEX_FLOAT_LITTLE_ENDIAN | Complex | 32 | Floating-point | Little Endian |
| COMPLEX FLOAT Big Endian (32f) | COMPLEX_FLOAT_BIG_ENDIAN | Complex | 32 | Floating-point | Big Endian |
| COMPLEX DOUBLE Little Endian (64fr) | COMPLEX_DOUBLE_LITTLE_ENDIAN | Complex | 64 | Floating-point | Little Endian |
| COMPLEX DOUBLE Big Endian (64f) | COMPLEX_DOUBLE_BIG_ENDIAN | Complex | 64 | Floating-point | Big Endian |

The desired output byte order is specified using the `output_bulkio_byte_order`
property, which defaults to host byte order. If the byte order of the input
file is different than that of the desired output byte order, byte swapping will
occur.

The `advanced_properties`, `default_timestamp`, and `default_sri` struct
properties, as well as the `default_sri_keywords` struct sequence property are
available to support more complicated use cases. Each of the features available
are documented in the description of each property when viewing the Properties
Descriptor XML file (i.e. FileReader.prf.xml).

## Copyrights

This work is protected by Copyright. Please refer to the
[Copyright File](COPYRIGHT) for updated copyright information.

## License

REDHAWK Basic Components rh.FileReader is licensed under the GNU General Public
License (GPL).
