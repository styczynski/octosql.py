from .storage import OctoSQLSource

class OctoSQLSourceJSON(OctoSQLSource):
    def __init__(self, name, path):
        OctoSQLSource.__init__(self, name)
        self.jsonPath = path

    def getConfig(self):
        return {
            "path": self.jsonPath
        }

    def getType(self):
        return "json"