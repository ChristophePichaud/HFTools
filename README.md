# HFTools

A cross-platform C++ library for database access with sample financial system implementation.

## Features

- **Generic Database Access Facade**: Abstract interface for database operations (openConnection, execQuery, getField, etc.)
- **Multiple Database Support**: 
  - PostgreSQL implementation
  - Sybase ASE implementation
- **Financial System Model**: Sample POCO classes for:
  - Users (traders, admins, analysts)
  - FX Instruments (currency pairs)
  - Trades (buy/sell transactions)
- **JSON Serialization**: Convert POCO objects to/from JSON using nlohmann/json
- **Sample Data**: DDL scripts and JSON data files for testing
- **Console Application**: Full-featured CLI with getopt command-line parsing

## Requirements

- CMake 3.12 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- nlohmann/json library (automatically downloaded if not found)

## Building

### Linux

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .

# The executable will be in build/hftools_app
```

### Windows

```bash
# Create build directory
mkdir build
cd build

# Configure (using Visual Studio generator)
cmake .. -G "Visual Studio 16 2019"

# Build
cmake --build . --config Release

# The executable will be in build/Release/hftools_app.exe
```

## Usage

### Run Test Demonstration

```bash
./hftools_app --test
```

This will demonstrate:
- JSON serialization/deserialization
- Database connection (PostgreSQL and Sybase)
- Loading JSON data files
- Executing sample queries

### Command-Line Options

```
Usage: hftools_app [OPTIONS]
Options:
  -d, --database TYPE     Database type (postgresql or sybase)
  -c, --connection STR    Connection string
  -q, --query QUERY       Execute SQL query
  -j, --json FILE         Load JSON file and display POCO objects
  -t, --test              Run test demonstration
  -h, --help              Display help message
```

### Examples

```bash
# Load and display users from JSON file
./hftools_app --json data/users.json

# Connect to PostgreSQL and run query
./hftools_app --database postgresql \
              --connection "host=localhost port=5432 dbname=hftools_db user=postgres" \
              --query "SELECT * FROM users"

# Connect to Sybase
./hftools_app --database sybase \
              --connection "server=localhost;database=hftools_db;user=sa"
```

## Project Structure

```
HFTools/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── include/
│   └── hftools/
│       ├── database/           # Database interface and implementations
│       │   ├── IDatabase.h
│       │   ├── Connection.h
│       │   ├── ResultSet.h
│       │   ├── PostgreSQLDatabase.h
│       │   └── SybaseDatabase.h
│       └── model/              # POCO classes
│           ├── User.h
│           ├── FXInstrument.h
│           └── Trade.h
├── src/
│   ├── database/               # Database implementations
│   ├── model/                  # POCO implementations
│   └── main.cpp               # Console application
└── data/
    ├── users.json             # Sample user data
    ├── fxinstruments.json     # Sample FX instrument data
    ├── trades.json            # Sample trade data
    ├── schema_postgresql.sql  # PostgreSQL DDL
    └── schema_sybase.sql      # Sybase DDL
```

## Database Setup

### PostgreSQL

```bash
# Create database
psql -U postgres -c "CREATE DATABASE hftools_db;"

# Load schema and sample data
psql -U postgres -d hftools_db -f data/schema_postgresql.sql
```

### Sybase ASE

```bash
# Connect as sa
isql -U sa -P password

# Load schema
isql -U sa -P password -i data/schema_sybase.sql
```

## JSON Data Format

### User
```json
{
  "id": 1,
  "username": "trader1",
  "email": "trader1@example.com",
  "role": "TRADER"
}
```

### FX Instrument
```json
{
  "id": 1,
  "symbol": "EUR/USD",
  "baseCurrency": "EUR",
  "quoteCurrency": "USD",
  "tickSize": 0.0001
}
```

### Trade
```json
{
  "id": 1,
  "userId": 1,
  "instrumentId": 1,
  "side": "BUY",
  "quantity": 100000.0,
  "price": 1.0850,
  "timestamp": "2024-01-28T10:30:00Z"
}
```

## Library API

### Database Access

```cpp
#include "hftools/database/PostgreSQLDatabase.h"

// Create database instance
auto db = std::make_shared<PostgreSQLDatabase>();

// Open connection
auto conn = db->openConnection("host=localhost port=5432 dbname=mydb user=postgres");

// Execute query
auto rs = conn->execQuery("SELECT * FROM users");

// Process results
while (rs->next()) {
    std::string username = rs->getField("username");
    std::string email = rs->getField("email");
    // ...
}

// Close connection
conn->close();
```

### JSON Serialization

```cpp
#include "hftools/model/User.h"

// Create object
User user(1, "trader1", "trader1@example.com", "TRADER");

// Serialize to JSON
nlohmann::json j = user.toJson();
std::cout << j.dump(2) << std::endl;

// Deserialize from JSON
nlohmann::json userJson = {
    {"id", 2},
    {"username", "admin1"},
    {"email", "admin1@example.com"},
    {"role", "ADMIN"}
};
User user2 = User::fromJson(userJson);
```

## License

This project is provided as-is for educational and development purposes.

## Notes

- The current implementation uses mock database connections for demonstration purposes
- To connect to real PostgreSQL/Sybase databases, link against libpq/sybdb libraries
- The database implementations provide a working interface that can be extended with actual client library calls