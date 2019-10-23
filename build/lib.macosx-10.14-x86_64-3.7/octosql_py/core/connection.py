import octosql_py_native
from .utils import GoString
from .query import OctoSQLQuery

class OctoSQLConnection:
    appID = -1

    def __init__(self, argAppID):
        self.appID = argAppID

    def createQuery(self, queryString):
        parseID = octosql_py_native.create_new_parse()

        octosql_py_native.parse(self.appID, parseID, queryString)
        octosql_py_native.plan(self.appID, parseID)
        return OctoSQLQuery(self.appID, parseID)