## Yato::Config

Yato Config is an universal facade for reading config of any origin. It provides single interface for arbitrary user defined backends, for example json, ini, etc.

Each config is a hierarchihal structure. 
Each node of config can be either *object* (key:value entries) or *array* (indexed values).
The stored value can be eitehr scalar (*integer*, *floating*, *boolean*, *string*) or child config node.

Read scalar values:

```c++

const yato::conf::config_ptr conf = /* backend dependent initialization */;

// answer is -1 if not found
int answer = conf->value<int>("answer").get_or(-1);

// throws excaption if not found
auto comment = conf->value<std::string>("comment").get();

float precision = conf->value<float>("precision").get_or(0.0f);

bool is_manual = conf->value<bool>("manual_mode").get_or(false);

```

Read array

```c++

// Returns nullptr if not found
yato::conf::config_ptr arr = conf->array("fruits");

std::cout << "My fruits:" << std::endl;
for(size_t i = 0; i < arr->size(); ++i) {
    std::cout << arr->value<std::string>(i).get() << std::endl;
}

```

Read nested config

```c++

// Returns nullptr if not found
yato::conf::config_ptr point = conf->config("location");
const int x = point->value<int>("x").get_or(-1);
const int y = point->value<int>("y").get_or(-1);

```


### Supported backends

#### Manual

Allows assembling config programmatically.

```c++

const auto conf = yato::conf::manual_builder::object()
    .put("answer", 42)
    .put("comment", "everything")
    .put("precision", 0.01f)
    .put("manual_mode", true)
    .put("fruits", yato::conf::manual_builder::array()
        .add("apple")
        .add("banana")
        .add("kiwi")
        .create())
    .put("location", yato::conf::manual_builder::object()
        .put("x", 174)
        .put("y", 34)
        .create())
    .create();

```

#### Json

```c++
#include <yato/config/json/json_config.h>

const char* json = R"JSON(
    {
        "answer": 42,
        "comment": "everything",
        "precision" : 0.01,
    
        "manual_mode" : true,

        "fruits" : [
            "apple", "banana", "kiwi"
        ],

        "location" : {
            "x" : 174,
            "y" : 34
        }
    }
)JSON";

const auto conf = yato::conf::json_builder().parse(json);

```

#### Command line

Command line config provides builder specifiying expected command line arguments and parsing command line into config.

Current implementation of the command line backend supports only plain object config. It can't have nested arrays or objects.

```c++

const yato::conf::config_ptr conf = yato::conf::cmd_builder("Test")
    .integer("", "answer", "integer argument with default value", yato::some(0))
    .string("c", "comment", "required string argument with one-letter alias")
    .floating("", "precision", "required floating-point argument")
    .boolean("", "manual_mode", "boolean flag")
    .parse(argc, argv);

```

