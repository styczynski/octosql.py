class OctoSQLSource:
    def __init__(self, name):
        self.sourceName = name

    def getConfig(self):
        pass

    def getType(self):
        return None

    def getName(self):
        return self.sourceName

    def generateConfigObject(self):
        return {
            "name": self.getName(),
            "type": self.getType(),
            "config": self.getConfig()
        }