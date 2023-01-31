DIFF_ADD = "#36CF70"
DIFF_REMOVE = "#A32929"


def to_hex_color(color):
    r, g, b = color["R"], color["G"], color["B"]
    return f"#{r:02X}{g:02X}{b:02X}"


def span_with_hint(text, hint):
    return f'<span title="{hint}">{text}</span>'


def colored_span(text, hex_color):
    return f'<span style="color:{hex_color}">{text}</span>'
