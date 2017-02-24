#!/usr/bin/env python3
import sys
import json


def main():
    with open(sys.argv[1]) as f:
        data = json.load(f)
        for register in data['c8y_Registers']:
            if 'number' not in register:
                print(register, "missing field 'number'")
            if 'input' not in register:
                print(register, "missing field 'input'")
            if 'startBit' not in register:
                print(register, "missing field 'startBit'")
            if 'noBits' not in register:
                print(register, "missing field 'noBits'")
            if 'multiplier' not in register:
                print(register, "missing field 'multiplier'")
            if 'divisor' not in register:
                print(register, "missing field 'divisor'")
            if 'decimalPlaces' not in register:
                print(register, "missing field 'decimalPlaces'")
            if 'signed' not in register:
                print(register, "missing field 'signed'")
            if 'statusMapping' in register:
                status = register['statusMapping']
                if 'status' not in status:
                    print(register, "statusMapping missing 'status'")
                elif status['status'] not in ('write', 'read'):
                    print(register, 'statusMapping invalid status')
            if 'alarmMapping' in register:
                alarm = register['alarmMapping']
                if 'type' not in alarm:
                    print(register, "alarmMapping missing 'type'")
                elif 'severity' not in alarm:
                    print(register, "alarmMapping missing 'severity'")
                elif 'text' not in alarm:
                    print(register, "alarmMapping missing 'text'")
            if 'measurementMapping' in register:
                measure = register['measurementMapping']
                if 'series' not in measure:
                    print(register, "measurementMapping missing 'series'")
                elif 'type' not in measure:
                    print(register, "measurementMapping missing 'type'")


if __name__ == '__main__':
    main()
