# NVS Rotative in Smart Energy Meters

NVS Rotative is a group of functions that allow the microcontroller ESP32 to use the API of NVS made by Espressif as a non-volatile circular memory buffer. 

## Installation

There is no package or API like yet, you need to copy the functions referred to this functionality. This functions are in main/mesh.c and they have these names: search_init_partition, contar_pulsos_nvs, levantar_bandera, change_to_next_partition,leer_pagina_variable, leer_contador_pf, abrir_pv, activa_entrada, guarda_cuenta_pulsos. Also you have to write a task similar to rotar_nvs.

## Explanation and Usage

Let's talk about the flash memory in ESP32, this is a part of the embedded memory which is able to save and remain data between restart or energy disconnections so when you turn on the ESP the next time, data will be there. Espressif developed an API called NVS to interact with this memory and make easier to handle it, even that separates or classifies the memory on three specific levels: Partitions, Pages, Entries.

### Partitions

### Pages

### Entries

```python
import foobar

foobar.pluralize('word') # returns 'words'
foobar.pluralize('goose') # returns 'geese'
foobar.singularize('phenomena') # returns 'phenomenon'
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
[MIT](https://choosealicense.com/licenses/mit/)