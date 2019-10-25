from octosql_py import octosql_py
from octosql_py.core.storage.json import OctoSQLSourceJSON
from octosql_py.core.storage.static import OctoSQLSourceStatic
import octosql_py_native

octo = octosql_py.OctoSQL()

conn = octo.connect([
    OctoSQLSourceStatic("lol", [
        { "a":99 }
    ]),
    OctoSQLSourceJSON("lol2", "./tests/samples/bikes.json"),
])
query = conn.createQuery("SELECT * FROM lol lol")
a = query.run()

print(a.values)