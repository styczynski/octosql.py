import octosql_py_native

class OctoSQLQuery:
    appID = -1
    parseID = -1

    def __init__(self, argAppID, argParseID):
        self.appID = argAppID
        self.parseID = argParseID

    def __del__(self):
        # octosql_py_native.delete_parse(self.parseID)
        pass

    def run(self):
        return octosql_py_native.run(self.appID, self.parseID)