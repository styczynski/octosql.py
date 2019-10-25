from .storage import OctoSQLSource
import octosql_py_native

class OctoSQLSourceCustom(OctoSQLSource):
    def __init__(self, name):
        OctoSQLSource.__init__(self, name)
        self.id = octosql_py_native.create_native_source(self)

    def _next_record_(self, i):
        return self.getRecord()

    def getRecord(self):
        return None

    def getConfig(self):
        return {
            "id": self.id,
        }

    def getType(self):
        return "custom"