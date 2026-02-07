Christophe, mon canard magnifique, allons-y.  
You want a **clean, modern, interface‑driven C++ database abstraction** that supports:

- **Sybase (DB-Lib)**  
- **PostgreSQL (libpq / pqxx / pqfe)**  
- A **common interface**: `IDBConnection`, `IDBReader`, `IDBValue`  
- A consistent extraction API:  
  ```cpp
  while (reader->next()) {
      int id;
      std::string name;
      (*reader) << id << name;
  }
  ```

Below is a **full, coherent, extensible architecture** you can drop into your project.  
It is intentionally modular so you can plug in your existing catalog + entity + repository system.

---

# 1. Common Interfaces (DB-agnostic)

```cpp
// IDatabase.hpp
#pragma once
#include <string>
#include <memory>

struct IDBValue {
    virtual ~IDBValue() = default;
    virtual bool isNull() const = 0;
    virtual int asInt() const = 0;
    virtual double asDouble() const = 0;
    virtual std::string asString() const = 0;
};

struct IDBRow {
    virtual ~IDBRow() = default;
    virtual size_t columnCount() const = 0;
    virtual const IDBValue& operator[](size_t idx) const = 0;

    // Sequential extraction
    template<typename T>
    IDBRow& operator<<(T& out) {
        assign(out);
        return *this;
    }

protected:
    virtual void assign(int& out) = 0;
    virtual void assign(double& out) = 0;
    virtual void assign(std::string& out) = 0;
};

struct IDBReader {
    virtual ~IDBReader() = default;
    virtual bool next() = 0;
    virtual IDBRow& row() = 0;
};

struct IDBConnection {
    virtual ~IDBConnection() = default;
    virtual std::unique_ptr<IDBReader> executeQuery(const std::string& sql) = 0;
};
```

This is your **portable foundation**.

---

# 2. PostgreSQL Implementation (using libpq-fe)

We implement:

- `PgValue`
- `PgRow`
- `PgReader`
- `PgConnection`

```cpp
// PgDatabase.hpp
#pragma once
#include "IDatabase.hpp"
#include <libpq-fe.h>
#include <vector>

class PgValue : public IDBValue {
public:
    PgValue(const char* v, bool null) : _null(null), _value(v ? v : "") {}

    bool isNull() const override { return _null; }
    int asInt() const override { return std::stoi(_value); }
    double asDouble() const override { return std::stod(_value); }
    std::string asString() const override { return _value; }

private:
    bool _null;
    std::string _value;
};

class PgRow : public IDBRow {
public:
    PgRow(PGresult* res, int row)
        : _res(res), _row(row), _cursor(0) {}

    size_t columnCount() const override {
        return PQnfields(_res);
    }

    const IDBValue& operator[](size_t idx) const override {
        bool isNull = PQgetisnull(_res, _row, idx);
        const char* v = isNull ? nullptr : PQgetvalue(_res, _row, idx);
        _cache.emplace_back(v, isNull);
        return _cache.back();
    }

protected:
    void assign(int& out) override {
        out = std::stoi(PQgetvalue(_res, _row, _cursor++));
    }

    void assign(double& out) override {
        out = std::stod(PQgetvalue(_res, _row, _cursor++));
    }

    void assign(std::string& out) override {
        char* v = PQgetvalue(_res, _row, _cursor++);
        out = v ? v : "";
    }

private:
    PGresult* _res;
    int _row;
    mutable std::vector<PgValue> _cache;
    size_t _cursor;
};

class PgReader : public IDBReader {
public:
    PgReader(PGresult* res) : _res(res), _row(-1) {}

    bool next() override {
        _row++;
        return _row < PQntuples(_res);
    }

    IDBRow& row() override {
        _current = std::make_unique<PgRow>(_res, _row);
        return *_current;
    }

    ~PgReader() {
        PQclear(_res);
    }

private:
    PGresult* _res;
    int _row;
    std::unique_ptr<PgRow> _current;
};

class PgConnection : public IDBConnection {
public:
    PgConnection(const std::string& conninfo) {
        _conn = PQconnectdb(conninfo.c_str());
        if (PQstatus(_conn) != CONNECTION_OK) {
            throw std::runtime_error(PQerrorMessage(_conn));
        }
    }

    ~PgConnection() {
        PQfinish(_conn);
    }

    std::unique_ptr<IDBReader> executeQuery(const std::string& sql) override {
        PGresult* res = PQexec(_conn, sql.c_str());
        if (PQresultStatus(res) != PGRES_TUPLES_OK &&
            PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::string err = PQerrorMessage(_conn);
            PQclear(res);
            throw std::runtime_error(err);
        }
        return std::make_unique<PgReader>(res);
    }

private:
    PGconn* _conn;
};
```

---

# 3. Sybase Implementation (DB-Lib)

You already have a DB-Lib wrapper.  
Here is the **interface-compliant version**:

```cpp
// SybaseDatabase.hpp
#pragma once
#include "IDatabase.hpp"
#include <sybfront.h>
#include <sybdb.h>
#include <vector>

class SybValue : public IDBValue {
public:
    SybValue(DBPROCESS* db, int col)
        : _db(db), _col(col) {}

    bool isNull() const override {
        return dbdata(_db, _col) == nullptr;
    }

    int asInt() const override {
        int v;
        memcpy(&v, dbdata(_db, _col), sizeof(int));
        return v;
    }

    double asDouble() const override {
        double v;
        memcpy(&v, dbdata(_db, _col), sizeof(double));
        return v;
    }

    std::string asString() const override {
        BYTE* d = (BYTE*)dbdata(_db, _col);
        int len = dbdatlen(_db, _col);
        return std::string((char*)d, len);
    }

private:
    DBPROCESS* _db;
    int _col;
};

class SybRow : public IDBRow {
public:
    SybRow(DBPROCESS* db) : _db(db), _cursor(1) {}

    size_t columnCount() const override {
        return dbnumcols(_db);
    }

    const IDBValue& operator[](size_t idx) const override {
        _cache.emplace_back(_db, idx + 1);
        return _cache.back();
    }

protected:
    void assign(int& out) override {
        out = SybValue(_db, _cursor++).asInt();
    }

    void assign(double& out) override {
        out = SybValue(_db, _cursor++).asDouble();
    }

    void assign(std::string& out) override {
        out = SybValue(_db, _cursor++).asString();
    }

private:
    DBPROCESS* _db;
    mutable std::vector<SybValue> _cache;
    int _cursor;
};

class SybReader : public IDBReader {
public:
    SybReader(DBPROCESS* db) : _db(db) {}

    bool next() override {
        int ret = dbnextrow(_db);
        return ret != NO_MORE_ROWS && ret != FAIL;
    }

    IDBRow& row() override {
        _current = std::make_unique<SybRow>(_db);
        return *_current;
    }

private:
    DBPROCESS* _db;
    std::unique_ptr<SybRow> _current;
};

class SybConnection : public IDBConnection {
public:
    SybConnection(const std::string& server,
                  const std::string& user,
                  const std::string& pwd,
                  const std::string& dbname)
    {
        dbinit();
        LOGINREC* login = dblogin();
        DBSETLUSER(login, (char*)user.c_str());
        DBSETLPWD(login, (char*)pwd.c_str());

        _db = dbopen(login, (char*)server.c_str());
        dbloginfree(login);

        dbuse(_db, (char*)dbname.c_str());
    }

    ~SybConnection() {
        dbclose(_db);
        dbexit();
    }

    std::unique_ptr<IDBReader> executeQuery(const std::string& sql) override {
        dbcmd(_db, (char*)sql.c_str());
        dbsqlexec(_db);
        dbresults(_db);
        return std::make_unique<SybReader>(_db);
    }

private:
    DBPROCESS* _db;
};
```

---

# 4. Usage (same code for both databases)

```cpp
std::unique_ptr<IDBConnection> conn;

// PostgreSQL
conn = std::make_unique<PgConnection>("host=... dbname=... user=...");

// Sybase
// conn = std::make_unique<SybConnection>("SERVER", "user", "pwd", "db");

auto reader = conn->executeQuery("select id, name from users");

while (reader->next()) {
    int id;
    std::string name;
    reader->row() << id << name;
}
```

Same code.  
Two databases.  
One duck.

---

# 5. What this gives you

### ✔ A unified DB abstraction  
### ✔ PostgreSQL + Sybase implementations  
### ✔ Sequential extraction with `operator<<`  
### ✔ Ready to plug into your Entity/Repository system  
### ✔ Clean, modern, interface‑driven architecture  

---

Christophe, if you want next:

- A **factory** that chooses Sybase/PostgreSQL at runtime  
- A **connection pool**  
- A **transaction API**  
- A **prepared statement API**  
- Integration with your **Catalog → EntityTraits → Repository** pipeline  

Just say the word and I’ll extend the ceremony.