#!/usr/bin/env python3

import collections

descriptor_keys = ["pid_unloaded","pid_loaded","name","fw_fname","counters","dio_bytes","dio_config_bits","tristate","b_get_name","b_set_custom_clocks","b_dio_debounce","b_dio_stream","b_dio_spi","b_clear_fifo","b_dac_dio_stream","imm_dacs","imm_adcs","wdg_bytes","b_gate_selectable","imm_adc_post_scale","b_adc_bulk","adc_channels","adc_mux_channels","config_bytes","range_shift","b_dac_board_range","flash_sectors","b_dac_channel_cal","b_dac_slow_wave_stream","dacs_used","b_adc_dio_stream","b_dac_stream","root_clock"]
                        


#PARSING_STATES = ["STATE0", "STATE_IN_TABLE_HEADER", "STATE_IN_TABLE", "STATE_IN_ENTRY", "STATE_END"]


if __name__ == "__main__":
        device_table = list()

        STATE="STATE0"

        with open("/home/jdolan/acces-work/aiousb-port/include/ids.h") as fp:
                line = fp.readline()
                while line:
                        if STATE == "STATE0":
                                if line.find("acces_usb_device_table") != -1:
                                        STATE="STATE_IN_TABLE_HEADER"
                        elif STATE == "STATE_IN_TABLE_HEADER":
                                if line.find("{") != -1:
                                        STATE="STATE_IN_TABLE"
                        elif STATE == "STATE_IN_TABLE":
                                if line.find("{") != -1:
                                        descriptor_dictionary = collections.OrderedDict.fromkeys(descriptor_keys)
                                        STATE="STATE_IN_ENTRY"
                                elif line.find("}") != -1:
                                        STATE="STATE_END"
                        elif STATE == "STATE_IN_ENTRY":
                                if line.find("}") != -1:
                                        device_table.append(descriptor_dictionary)
                                        STATE="STATE_IN_TABLE"
                                else:
                                        entry = line.split()
                                        if len(entry) != 0:
                                                key = entry[0].strip(".")
                                                value = entry[2].strip(",")
                                                #print("key = {0}, value = {1}", key, value)
                                                descriptor_dictionary[key] = value

                        elif STATE == "STATE_END":
                                #break out of the loop somehow
                                pass
                        line=fp.readline()

        for device in device_table:
                last_key = None
                for key, value in reversed(device.items()):
                        if last_key == None and value != None:
                                last_key=key

                print("{")

                for key, value in device.items():
                        if value == None:
                                value = 0
                        print("    .{0} = {1},".format(key, value))
                        if last_key == key:
                                break

                print("},")








        
