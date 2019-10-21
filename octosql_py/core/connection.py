from .utils import GoString
from .query import OctoSQLQuery

class OctoSQLConnection:
    lib = None
    appID = -1

    def __init__(self, argLib, argAppID):
        self.lib = argLib
        self.appID = argAppID

    def createQuery(self, queryString):
        parseID = self.lib.octosql_create_new_parse()

        wrappedQuery = GoString(queryString.encode(), len(queryString))
        self.lib.octosql_parse(self.appID, parseID, wrappedQuery)
        self.lib.octosql_plan(self.appID, parseID)
        return OctoSQLQuery(self.lib, self.appID, parseID)