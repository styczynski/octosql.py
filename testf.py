from octosql_py import octosql_py
from octosql_py.core.storage.json import OctoSQLSourceJSON

octo = octosql_py.OctoSQL()
conn = octo.connect([
    OctoSQLSourceJSON("lol", "./bikes.json")
])
query = conn.createQuery("SELECT * FROM lol lol")
query.run()
