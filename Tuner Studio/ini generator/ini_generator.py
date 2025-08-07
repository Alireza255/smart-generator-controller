import re


# Add this global mapping dictionary:
C_TO_INI_TYPE = {
    'float': 'F32',
    'uint32_t': 'U32',
    'int32_t': 'I32',   # if you want to treat signed same as unsigned here
    'uint16_t': 'U16',
    'int16_t': 'I16',
    'uint8_t': 'U8',
    'int8_t': 'I8',
    # add more if needed
}



# ------------------
# Constants Parser
# ------------------
def parse_constants_file(constants_file):
    constants = {}
    with open(constants_file, 'r') as file:
        lines = file.readlines()

    for line in lines:
        line = line.strip()
        if line.startswith('#define'):
            tokens = line.split()
            if len(tokens) >= 3:
                name = tokens[1]
                value = tokens[2]

                # Extract number even if it's like (type)123
                numeric_match = re.search(r"(-?\d+)", value)
                if numeric_match:
                    constants[name] = int(numeric_match.group(1))
                # else ignore non-numeric macros
    return constants

# ------------------
# Struct Parser
# ------------------
TYPE_SIZES = {
    'uint8_t': 1,
    'int8_t': 1,
    'uint16_t': 2,
    'int16_t': 2,
    'uint32_t': 4,
    'int32_t': 4,
    'float': 4,
    'double': 8,
    'char': 1,
    'int': 4,  # Assuming 32-bit
}

def parse_struct(file_path, struct_name, constants_dict):
    with open(file_path, 'r') as file:
        content = file.read()

    # Try anonymous struct
    struct_pattern = rf"typedef\s+struct\s*\{{(.*?)\}}\s*{struct_name}\s*;"
    match = re.search(struct_pattern, content, re.DOTALL)

    # Try tagged struct if anonymous not found
    if not match:
        struct_pattern_tagged = rf"typedef\s+struct\s+\w+\s*\{{(.*?)\}}\s*{struct_name}\s*;"
        match = re.search(struct_pattern_tagged, content, re.DOTALL)

    if not match:
        raise ValueError(f"Struct '{struct_name}' not found in {file_path}")

    struct_body = match.group(1)

    # Extract fields: type, name, arrays (allow multiple dimensions)
    field_pattern = r"\s*([\w\s\*]+)\s+(\w+)(\s*(\[[^\]]+\])*)\s*;"
    fields = re.findall(field_pattern, struct_body)

    parsed_fields = []
    for field in fields:
        field_type = field[0].strip()
        field_name = field[1].strip()
        array_sizes_str = field[2]

        sizes = []
        if array_sizes_str:
            size_tokens = re.findall(r"\[([^\]]+)\]", array_sizes_str)
            for token in size_tokens:
                token = token.strip()
                if token.isdigit():
                    sizes.append(int(token))
                elif token in constants_dict:
                    sizes.append(constants_dict[token])
                else:
                    raise ValueError(f"Unknown constant/macro: {token}")

        parsed_fields.append((field_type, field_name, sizes))

    return parsed_fields

# ------------------
# Size Calculator
# ------------------
def compute_size(field_type, array_sizes):
    base_size = TYPE_SIZES.get(field_type.strip(), None)
    if base_size is None:
        raise ValueError(f"Unknown type '{field_type}'")

    total_elements = 1
    for size in array_sizes:
        total_elements *= size

    return base_size * total_elements

# ------------------
# INI Generator
# ------------------


def generate_ini(struct_fields, ini_template, output_file):
    with open(ini_template, 'r') as file:
        lines = file.readlines()

    new_lines = []
    inserted = False
    offset_counter = 0

    for line in lines:
        new_lines.append(line)
        # Insert after [Constants] section header
        if line.strip().lower() == "[constants]" and not inserted:
            new_lines.append("\n; --- Auto-generated from Struct ---\n")
            for field_type, field_name, array_sizes in struct_fields:
                size_in_bytes = compute_size(field_type, array_sizes)
                offset_dec = f"{offset_counter}"

                shape_str = ''
                if array_sizes:
                    shape_str = ''.join([f'[{s}]' for s in array_sizes])
                ini_type = C_TO_INI_TYPE.get(field_type.strip(), field_type)  # fallback to original if missing
                ini_line = f"{field_name} = scalar, {ini_type}, {offset_dec}, {shape_str}, , 1, 0, 0, 100, 0\n"
                new_lines.append(ini_line)

                offset_counter += size_in_bytes

            inserted = True

    with open(output_file, 'w') as file:
        file.writelines(new_lines)

# ------------------
# Main Runner
# ------------------
if __name__ == "__main__":
    constants = parse_constants_file('constants.h')
    struct_fields = parse_struct('input.h', 'engine_configuration_t', constants)
    generate_ini(struct_fields, 'config.ini', 'output_config.ini')
    print("INI file generated with macros resolved! Check 'output_config.ini'")
