from octosql_py import octosql_py
from octosql_py.core.storage.json import OctoSQLSourceJSON
import octosql_py_native


octo = octosql_py.OctoSQL()
conn = octo.connect([
    OctoSQLSourceJSON("lol", "./bikes.json")
])
query = conn.createQuery("SELECT * FROM lol lol")
a = query.run()
print(a[0]["color"])