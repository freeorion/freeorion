from freeorion_tools import get_ai_tag_grade


def test_parse_weapon_tag_positive1():
    assert 'GREAT' == get_ai_tag_grade(['GREAT_WEAPONS'], 'WEAPONS')


def test_parse_weapon_tag_positive2():
    assert 'GREAT' == get_ai_tag_grade(['GREAT_MULTI_WORD_TAG'], 'MULTI_WORD_TAG')


def test_parse_weapon_tag_negative1():
    assert '' == get_ai_tag_grade(['WEAPONS'], 'WEAPONS')


def test_parse_weapon_tag_negative2():
    assert '' == get_ai_tag_grade(['GREAT_WEAPONS'], 'WEAPON')
