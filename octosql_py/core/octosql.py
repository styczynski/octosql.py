import octosql_py_native
import yaml

from .connection import OctoSQLConnection

class OctoSQL:

    def __init__(self):
        octosql_py_native.init()

    def connect(self, sources):
        conf = {
            "dataSources": list(map(lambda source: source.generateConfigObject(), sources))
        }
        config = yaml.dump(conf)
        appID = octosql_py_native.new_instance(config)
        return OctoSQLConnection(appID)