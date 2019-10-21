from .octosql import GoString

class OctoSQLQuery:
    lib = None
    appID = -1
    parseID = -1

    def __init__(self, argLib, argAppID, argParseID):
        self.lib = argLib
        self.appID = argAppID
        self.parseID = argParseID

    def run(self):
        return self.lib.octosql_run(self.appID, self.parseID)