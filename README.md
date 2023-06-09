# Simple Sample Driver for Xentara
This project contains a simple sample driver for Xentara. It requires the Xentara development environment, as well as a Xentara licence. You can get a Xentara licence in the [Xentara Online Shop](https://www.xentara.io/product/xentara-for-industrial-automation/).

The documentation for Xentara can be found at https://docs.xentara.io/xentara

This driver used the Xentara Utility Llibrary, as well as the Xentara Plugin Framework. Docs can be found here:

- https://docs.xentara.io/xentara-utils/
- https://docs.xentara.io/xentara-plugin/

## Functionality
This driver reads and writes files in the user's home directory. Different files in different directories can be written and read.

## Xentara Elements
This driver provides three types of Xentara elements, described below.

### Devices
The driver supplies an [I/O component](https://docs.xentara.io/xentara/xentara_io_components.html) with model file descriptor
`@Skill.SimpleSampleDriver.Device`, that represents a directory under the user’s home directory. This component has no
functionality in its own and only serves as a container for the inputs and outputs.

The class can be found in the following files:

- [src/Device.hpp](src/Device.hpp)
- [src/Device.cpp](src/Device.cpp)

The *Device* class publishes the following [attribute](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_attributes):

- `directory` contains the absolute path of the directory.

The device class has no events or tasks.

### Inputs
The driver supplies a [skill data point](https://docs.xentara.io/xentara/xentara_skill_data_points.html) with model file descriptor
`@Skill.SimpleSampleDriver.Input`, that reads a double precision floating point value from a text file.

The class can be found in the following files:

- [src/Input.hpp](src/Input.hpp)
- [src/Input.cpp](src/Input.cpp)

The *Input* class publishes the following [attributes](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_attributes):

- `directory` contains the absolute path of the enclosing directory. This attribute is inherited fron the device.
- `fileName` contains the file name within the directory.
- `updateTime` contains the last time the file was read.
- `value` contains the value that was read.
- `changeTime` contains the last time the value changed.
- `quality` contains the [quality](https://docs.xentara.io/xentara/xentara_quality.html) of the value.
- `error` contains an error code if the file could not be read.

The *Input* class published the following [events](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_events):

- `value` is triggered whenever the value changes
- `quality` is triggered whenever the [quality](https://docs.xentara.io/xentara/xentara_quality.html) changes
- `changed` is trigger whenever anything changes

The *Input* class published the following [tasks](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_tasks):

- `read` reads the file and updates the value.

### Outputs
The driver supplies a [skill data point](https://docs.xentara.io/xentara/xentara_skill_data_points.html) with model file descriptor
`@Skill.SimpleSampleDriver.Output`, that writes a double precision floating point value to a text file.

The class can be found in the following files:

- [src/Output.hpp](src/Output.hpp)
- [src/Output.cpp](src/Output.cpp)

The *Output* class publishes the following [attributes](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_attributes):

- `directory` contains the absolute path of the enclosing directory. This attribute is inherited fron the device.
- `fileName` contains the file name within the directory.
- `value` is a write-only attribute that can be written to write to the file.
- `writeTime` contains the last time the file was written, or attempted to be written.
- `writeError` contains an error code if the file could not be written.

The *Output* class published the following [events](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_events):

- `written` is triggered whenever the file was written successfully.
- `writeError` is triggered whenever an error occurred writing the file.

The *Output* class published the following [tasks](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_tasks):

- `write` writes the last value written to the `value` attribute out to the file.

## The Sample Model
This project contains a sample model file [config/model.json](config/model.json). The file contains the following functionality:

1. Reads the contents of two files, *F1.txt* and *F2.txt* from the sub-directory *Xentara/Data/D1* of the user’s home directory.
  The files are read once a second, and the read value is printed to standard out if it changed.
2. Writes a sine wave to the file *F1.txt* in the sub-directory *Xentara/Data/D2* of the user’s home directory. The file is
   written once a second. Successful and failed writes are logged to standard out.
3. Writes a pulse wave to the file *F2.txt* in the sub-directory *Xentara/Data/D2* of the user’s home directory. The file is
   written once a second, but only if the value has changed.  Successful and failed writes are logged to standard out.
4. Reads the two written files back in the same way as the files from directory *Xentara/Data/D1*.
