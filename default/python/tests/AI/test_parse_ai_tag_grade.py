from freeorion_tools import get_ai_tag_grade


def test_parse_weapon_tag_positive():
    assert 'GREAT' == get_ai_tag_grade(['AI_TAG_GREAT_WEAPONS'], 'WEAPONS')


def test_parse_weapon_tag_negative():
    assert '' == get_ai_tag_grade(['AI_TAG_WEAPONS'], 'WEAPONS')

