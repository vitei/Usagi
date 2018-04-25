import unittest
import EntityYaml

class Test_EntityYamlTest(unittest.TestCase):
    def test_A(self):
        entity = EntityYaml.EntityYaml('Entities/Base.yml')

if __name__ == '__main__':
    unittest.main()
