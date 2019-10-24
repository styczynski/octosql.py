from .storage import OctoSQLSource

class OctoSQLSourceNative(OctoSQLSource):
    def __init__(self, name):
        OctoSQLSource.__init__(self, name)

    def getConfig(self):
        return {}

    def getType(self):
        return "custom"