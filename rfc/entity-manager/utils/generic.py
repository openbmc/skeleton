import StringIO
import json
from string import Template


def prep_json(file):
    new_file = StringIO.StringIO()
    for line in file:
        if not line.lstrip().startswith('//'):
            new_file.write(line)
    new_file.seek(0)
    return new_file


def dict_template_replace(d, temp):
    replaced = Template(json.dumps(d)).substitute(temp)
    return json.loads(replaced)
