from octosql_py import octosql_py
from octosql_py.core.storage.json import OctoSQLSourceJSON
from octosql_py.core.storage.native import OctoSQLSourceNative
import octosql_py_native

def fact(a):
    return {
        "kk": 109,
        "lol": "asd"
    }

octo = octosql_py.OctoSQL()
octosql_py_native.create_native_source(fact)

conn = octo.connect([
    OctoSQLSourceNative("lol")
])
query = conn.createQuery("SELECT * FROM lol lol")
a = query.run()

print(a.values)

# conn = octo.connect([
#     OctoSQLSourceJSON("lol", "./tests/samples/bikes.json")
# ])
# query = conn.createQuery("SELECT * FROM lol lol")
# a = query.run()
#
# print(a.values)