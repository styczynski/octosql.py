#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Tests for `octosql_py` package."""

import pytest

from click.testing import CliRunner

from octosql_py import octosql_py
from octosql_py import cli
from octosql_py.core.storage.json import OctoSQLSourceJSON

@pytest.fixture
def response():
    """Sample pytest fixture.

    See more at: http://doc.pytest.org/en/latest/fixture.html
    """

def test_command_line_interface():
    """Test the CLI."""
    runner = CliRunner()
    result = runner.invoke(cli.main)
    assert result.exit_code == 0
    assert 'octosql_py.cli.main' in result.output
    help_result = runner.invoke(cli.main, ['--help'])
    assert help_result.exit_code == 0
    assert '--help  Show this message and exit.' in help_result.output
