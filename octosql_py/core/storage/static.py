from .custom import OctoSQLSourceCustom

class OctoSQLSourceStatic(OctoSQLSourceCustom):
    def __init__(self, name, val):
        OctoSQLSourceCustom.__init__(self, name)
        self.val = val
        self.index = 0

    def getRecord(self):
        if self.index >= len(self.val):
            return None
        record = self.val[self.index]
        self.index = self.index + 1
        return record