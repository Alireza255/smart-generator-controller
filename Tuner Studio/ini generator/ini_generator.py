
import re
import sys

# Mapping of C types to TunerStudio types and sizes (in bytes)
TYPE_MAP = {
    "float": ("F32", 4),
    "uint16_t": ("U16", 2),
    "uint8_t": ("U08", 1),
    "bool": ("U08", 1),  # Treat bool as 1 byte in packed structure
}

# Units for TunerStudio output
UNITS_MAP = {
    "rpm": ("RPM", 1, 0),
    "map": ("kPa", 0.01, 0),
    "tps": ("%", 0.01, 0),
    "lambda": ("", 0.0001, 0),
    "advance": ("deg", 0.02, 0),
    "dwell": ("ms", 1, 0),
    "clt": ("C", 0.001, 0),
    "vbatt": ("V", 0.001, 0),
    "sync_loss_count": ("", 1, 0),
    "iat": ("C", 0.001, 0),
    "ego_correction": ("%", 1, 0),
    "warmup_enrichment": ("%", 1, 0),
    "acceleration_enrichment": ("%", 1, 0),
    "gamma_enrichment": ("%", 1, 0),
    "ve1": ("%", 1, 0),
    "ve2": ("%", 1, 0),
    "lambda_target": ("", 1, 0),
    "tps_adc_value": ("", 1, 0),
    "injector_pulse_width": ("ms", 1, 0),
    "gas_valve_position": ("%", 1, 0),
    "target_rpm": ("RPM", 1, 0),
    "fuel_pressure_gas": ("kPa", 1, 0),
    "fuel_pressure_petrol": ("kPa", 1, 0),
    "spark_per_ignition_event_count": ("", 1, 0),
    "fuel_load_gas": ("%", 1, 0),
    "fuel_load_petrol": ("%", 1, 0),
}

def main():
    print("Script started")
    if len(sys.argv) != 2:
        print("Usage: python ini_generator.py <path_to_comms.h>")
        sys.exit(1)

    file_path = sys.argv[1]
    print(f"Attempting to read file: {file_path}")
    try:
        with open(file_path, 'r') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found")
        sys.exit(1)
    print("File read successfully")

    # Match only output_channels_t structure
    match = re.search(r'typedef struct\s*(?:__attribute__$$ \(packed $$\)\s*)?\{(.*?)\}\s*output_channels_t\s*;', content, re.DOTALL)
    if not match:
        print("Error: Could not find 'output_channels_t' structure")
        sys.exit(1)
    struct_content = match.group(1)
    lines = [line.strip() for line in struct_content.split(';') if line.strip()]
    print(f"Found {len(lines)} lines in output_channels_t structure")

    config_lines = []
    offset = 0  # Cumulative byte offset

    for line in lines:
        line = re.sub(r'//.*', '', line).strip()  # Remove comments
        line = re.sub(r'__attribute__$$ \(packed $$\)', '', line).strip()  # Remove __attribute__((packed))
        if not line:
            print("Skipping empty line")
            continue
        match = re.match(r'(\w+)\s+([\w$$  $$]+)', line)
        if not match:
            print(f"Skipping unparsable line: {line}")
            continue
        type_name, name = match.groups()
        size = 1
        if '[' in name:
            name, size_str = name.split('[')
            size_str = size_str.rstrip(']')
            if not size_str.isdigit():
                print(f"Skipping array with non-numeric size: {name}[{size_str}]")
                continue
            size = int(size_str)
            print(f"Array detected: {name}[{size}]")

        if type_name not in TYPE_MAP:
            print(f"Skipping unsupported type: {type_name}")
            continue
        ts_type, type_size = TYPE_MAP[type_name]
        print(f"Processing {name}: Type {type_name}, TunerStudio type {ts_type}, Size {type_size} bytes")

        units, scale, translate = UNITS_MAP.get(name, ("", 1, 0))
        for i in range(size):
            field_name = f"{name}{i if size > 1 else ''}"
            if type_name == "bool":
                config_lines.append(f"status_{field_name} = scalar, {ts_type}, {offset}, \"\", 1, 0")
            else:
                config_lines.append(f"{field_name}_value = scalar, {ts_type}, {offset}, \"{units}\", {scale}, {translate}")
            print(f"Added field: {field_name} at offset {offset}, units: {units}")
            offset += type_size

    print(f"Generated {len(config_lines)} configuration lines")
    print("[OutputChannels]")
    for line in config_lines:
        print(line)
    print(f"Total structure size: {offset} bytes")
    print("Script completed successfully")

if __name__ == "__main__":
    main()