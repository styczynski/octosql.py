#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Tests for `octosql_py` package: CLI interface """

import pytest

from octosql_py import octosql_py
from octosql_py.core.storage.json import OctoSQLSourceJSON

@pytest.fixture
def response():
    """Basic octosql tests."""

def test_basic_json(response):
    """Sample pytest test function with the pytest fixture as an argument."""
    octo = octosql_py.OctoSQL()
    conn = octo.connect([
        OctoSQLSourceJSON("lol", "./tests/samples/bikes.json")
    ])
    query = conn.createQuery("SELECT * FROM lol lol")
    a = query.run()

    valid_result = [
       {'lol.color': 'green', 'lol.id': 1.0, 'lol.ownerid': 152849.0, 'lol.wheels': 3.0, 'lol.year': 2014.0},
       {'lol.color': 'black', 'lol.id': 2.0, 'lol.ownerid': 106332.0, 'lol.wheels': 2.0, 'lol.year': 1988.0},
       {'lol.color': 'purple', 'lol.id': 3.0, 'lol.ownerid': 99148.0, 'lol.wheels': 2.0, 'lol.year': 2009.0},
       {'lol.color': 'orange', 'lol.id': 4.0, 'lol.ownerid': 97521.0, 'lol.wheels': 2.0, 'lol.year': 1979.0}
    ]
    assert sorted(a.values, key=lambda x: [ [k,v] for k, v in x.items() ]) == sorted(valid_result, key=lambda x: [ [k,v] for k, v in x.items() ])
