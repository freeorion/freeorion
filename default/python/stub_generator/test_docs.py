from stub_generator.parse_docs import Docs


def test_multiple_declaration_with_docstrings():
    text = """
getEmpire() -> empire :
    Returns B

getEmpire( (int)arg1) -> empire :
    Returns A
""".strip(
        "\n"
    )

    docs = Docs(text, indent=0, is_class=False)
    assert list(docs.get_argument_strings()) == ["", "number: int"]
    assert docs.get_doc_string() == ('"""\nReturns B\nReturns A\n"""')


def test_single_declaration_with_docstrings():
    text = """
empirePlayerID( (int)arg1) -> int :
    Returns ...
    """.strip(
        "\n"
    )
    docs = Docs(text, indent=0, is_class=False)
    assert list(docs.get_argument_strings()) == ["number: int"]
    assert docs.get_doc_string() == '"""\nReturns ...\n"""'


def test_docs_single_line():
    text = "inQueue( (researchQueue)arg1, (str)arg2) -> bool"
    docs = Docs(text, indent=0, is_class=False)
    assert list(docs.get_argument_strings()) == ["research_queue: researchQueue, string: str"]
    assert docs.get_doc_string() == ""
