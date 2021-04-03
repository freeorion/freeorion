from EnumsAI import Enum, EnumItem


class MyEnum(Enum):
    ONE = 1
    TWO = 2


def test_enum_attribute_type_is_enum_item():
    assert isinstance(MyEnum.ONE, EnumItem)
    assert isinstance(MyEnum.ONE, int)


def test_enum_item_is_equals_to_value():
    assert MyEnum.ONE == 1


def test_has_item_accepts_enum_value_of_this_enum():
    assert MyEnum.has_item(MyEnum.ONE)


def test_has_item_rejects_item_of_other_enum():
    class OtherEnum(Enum):
        ONE = 1

    assert MyEnum.has_item(OtherEnum.ONE) is False


def test_enum_item_attributes():
    assert MyEnum.ONE.name == "ONE"
    assert MyEnum.ONE.enum_name == "MyEnum"
