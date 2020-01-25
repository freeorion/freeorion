import unittest


class PropertyTester(unittest.TestCase):
    class_to_test = None
    properties = {}
    longMessage = True

    TYPE = "type"
    READ_ONLY = "read_only"
    ADDITIONAL_TESTS = "additional_tests"

    @classmethod
    def generate_tests(cls):
        for property in cls.properties:
            setattr(cls, "test_property_%s" % property,
                    PropertyTester._test_case_generator(property, cls.properties[property]))

    @staticmethod
    def _test_case_generator(property, kwargs):
        def _test_case(self):
            for obj in self.objects_to_test:
                self.__test_property(property, obj, **kwargs)
        return _test_case

    def __test_property(self, property_to_test, obj, type=None, read_only=True, additional_tests=[]):
        self.assertIn(property_to_test, dir(obj))
        property_to_test = getattr(self.class_to_test, property_to_test)
        return_value = property_to_test.fget(obj)
        if type is not None:
            self.assertIsInstance(return_value, type)
        else:
            self.assertIsNone(return_value)

        if read_only:
            self.assertIsNone(property_to_test.fset)
        else:
            property_to_test.fset(obj, type())
            self.assertEqual(property_to_test.fget(obj), type())
            property_to_test.fset(obj, return_value)
            self.assertEqual(property_to_test.fget(obj), return_value)

        for test in additional_tests:
            test[0](self, return_value, *test[1:])


class DumpTester(unittest.TestCase):

    def test_dump(self):
        for obj in self.objects_to_test:
            retval = obj.dump()
            self.assertIsInstance(retval, str)

    def test_dump_takes_no_args(self):
        for obj in self.objects_to_test:
            for arg in (int(), float(), str(), None):
                self.assertRaises(Exception, obj.dump, arg)

    def setUp(self):
        self.objects_to_test = []
        raise NotImplementedError
