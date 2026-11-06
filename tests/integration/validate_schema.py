#!/usr/bin/env python3
"""Checks docs/performance.schema.json is a valid schema, the example validates,
and obviously bad Performances are rejected."""
import copy, json, pathlib, sys

try:
    from jsonschema import Draft202012Validator
except ImportError:
    print("missing dependency: python3 -m pip install jsonschema")
    sys.exit(2)

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
schema = json.loads((ROOT / "docs" / "performance.schema.json").read_text(encoding="utf-8"))
example = json.loads((ROOT / "docs" / "example.performance.json").read_text(encoding="utf-8"))

Draft202012Validator.check_schema(schema)
v = Draft202012Validator(schema)
fails = 0

def expect(label, doc, valid):
    global fails
    errs = list(v.iter_errors(doc))
    ok = (not errs) == valid
    print(("ok   " if ok else "FAIL ") + label + ("" if ok else f" -> {errs[0].message if errs else 'accepted'}"))
    fails += 0 if ok else 1

expect("example validates", example, True)

d = copy.deepcopy(example); d["parts"] = [dict(example["parts"][0], id=1 + i % 16) for i in range(17)]
expect("17 parts rejected", d, False)

d = copy.deepcopy(example); d["scenes"] = [example["scenes"][0]] * 9
expect("9 scenes rejected", d, False)

d = copy.deepcopy(example); d["parts"][0]["pan"] = 2
expect("pan 2 rejected", d, False)

d = copy.deepcopy(example); d["parts"][0]["velocityCurve"] = "crazy"
expect("bad velocity curve rejected", d, False)

d = copy.deepcopy(example); d["scenes"][0]["parts"]["17"] = {"enabled": True}
expect("scene part id 17 rejected", d, False)

d = copy.deepcopy(example); d["schemaVersion"] = 2
expect("wrong schemaVersion rejected", d, False)

sys.exit(1 if fails else 0)
