import tkinter as tk
from tkinter import ttk
from ttkbootstrap import Style
import time
import serial
import xlwings as xw
import os
from tkinter.filedialog import askdirectory

style = Style(theme='sandstone')

button_start_state = False
button_stop_state = False
serial_state = False
#root = tk.Tk()
root = style.master

root.title("5-CSRTT Control Pannel")
root.geometry("800x400")
root.minsize(400,200)

serial_input_dict = {'Correct':0,'Incorrect':0,'Omission':0,'Total':0}
serial_input_dict_copy = {'Correct':0,'Incorrect':0,'Omission':0,'Total':0}
cell_num = 1
final_data_num = 2
time_start = time.time()
time_end = time.time()


# Create the file to store data
app = xw.App(visible=False, add_book=False)
wb = app.books.add()
sheet_log = wb.sheets.add("Log")
sheet_result = wb.sheets.add("result")
wb.sheets["Sheet1"].delete()
sheet_result.range("B1").value = time.strftime('%Y.%m.%d %H:%M:%S')

#Current time
def refresh():
    global time_duration
    global time_start
    global time_end
    global button_start_state
    global button_stop_state
    #Display current time
    var = tk.StringVar()
    var = time.strftime('%H:%M:%S')
    label_time = tk.Label(root, text=var, width=40, anchor='w', font=("Arial", 20)).place(x=660, y=20)

    
    if button_stop_state == False and button_start_state == True:

        time_end = time.time()
        
    #Duration of the trial
        try:
            time_duration = time_end - time_start
            time_duration = round(time_duration)

            # If the time is over, stop the experiment
            if time_limit_var.get() != "" and time_duration >= int(time_limit_var.get()):
                warning_message("time up" + "No" + output_trial_num.get() + "Trail finished")
                button_stop_state = True
                button_start_state = False

            label_time1 = tk.Label(root, text=time_duration, width=10, anchor='w', font=("Arial", 20)).place(x=560, y=50)
            label_time2 = tk.Label(root, text="s", width=10, anchor='w',font=("Arial", 20)).place(x=610, y=50)
        except:
                pass


        # Attempt to read serial port
        try:
            port_input_filter()  # Activate serial port reception
            label_result()  #Display the data received by the serial port
            data_output() # Write in the data to the file
        except:
            pass
    elif button_stop_state == True and button_start_state == False:
        time_duration = time_end - time_start
        time_duration = round(time_duration)
        pass


    root.after(1000,refresh)



def button_start_fun():
    global time_start
    global button_stop_state
    global serial_state
    global ser
    global button_start_state
    button_start_state = True
    button_start = tk.Button(root, text="Start", height=2, width=15, font=("Arial", 12), command=button_start_fun,
                            state='disabled').place(x=150, y=200)
    button_stop = tk.Button(root,text="Stop",height = 2, width = 15, font=("Arial", 12), command = button_stop_fun,
                            state = 'normal').place(x=350, y=200)


    #Label displays the experiment number entered
    label_output = tk.Label(root, text=output_trial_num.get(), width=15, anchor='w', font=("Arial", 20)).place(x=560, y=80)
    
    #Start timing
    if len(trial_var.get()) == 0: #Check if there is invalid input in the experiment name input box. If not, a warning message will be prompted
        pass
        warning_message("Please enter the trail No. before starting!")
    else:
        warning_message("")

    # Configure serial port parameters
    if serial_state == True: 
        ser.close()
    try:
        serial_num = port_entry.get()
        ser = serial.Serial('COM' + str(serial_num), 9600, timeout=1)
        ser.write("s") #Send the start command to Arduino
        time_start = time.time()
        button_stop_state = False  
        serial_state = True
    except:
        warning_message("Wrong port number")


def button_stop_fun():
    #Stop timing
    global button_start_state
    global button_stop_state
    global time_end
    button_start_state = False  
    button_stop_state = True
    # ser.write("stop") ##Send the stop command to Arduino
    time_end = time.time()
    final_data()
    button_start = tk.Button(root, text="Start", height=2, width=15, font=("Arial", 12), command=button_start_fun,
                            state='normal').place(x=150, y=200)
    button_stop = tk.Button(root,text="Stop",height = 2, width = 15, font=("Arial", 12), command = button_stop_fun,
                            state = 'disabled').place(x=350, y=200)

def button_clear_fun():
    global serial_input_dict
    label_correct_num = tk.Label(root, text="0", width=5, anchor='w', font=("Arial", 20)).place(x=260, y=20)
    label_wrong_num = tk.Label(root, text="0", width=5, anchor='w', font=("Arial", 20)).place(x=260, y=50)
    label_ommision_num = tk.Label(root, text="0", width=5, anchor='w', font=("Arial", 20)).place(x=260, y=80)
    label_total_num = tk.Label(root, text="0", width=5, anchor='w', font=("Arial", 20)).place(x=260, y=110)
    label_time1 = tk.Label(root, text="0", width=2, anchor='w', font=("Arial", 20)).place(x=560, y=50)
    serial_input_dict = {'Correct': 0, 'Incorrect': 0, 'Omission': 0, 'Total': 0}
    warning_message("")

def port_input_filter():
    port_input = ser.readline().decode()
    content = port_input.split(":")  
    if content[0].strip() in serial_input_dict.keys():
        serial_input_dict[content[0]] = content[-1].strip()


# Display trial parameters
def label_result():
    label_correct_num = tk.Label(root, text=serial_input_dict['Correct'], width=5, anchor='w',font=("Arial", 20)).place(x=260, y=20)
    label_wrong_num = tk.Label(root, text=serial_input_dict['Incorrect'], width=5, anchor='w', font=("Arial", 20)).place(x=260, y=50)
    label_ommision_num = tk.Label(root, text=serial_input_dict['Omission'], width=5, anchor='w', font=("Arial", 20)).place(x=260, y=80)
    label_total_num = tk.Label(root, text=serial_input_dict['Total'], width=5, anchor='w', font=("Arial", 20)).place(x=260, y=110)



def warning_message(message):
    label_warning = tk.Label(root, text= message, fg = 'Red',width=30, anchor='center', font=("Arial", 25)).place(x=140, y=150)


def data_output():
    #Write data to an Excel file
    global cell_num
    sheet_log.range("A1").value = "Number"
    sheet_log.range("B1").value = "time"
    sheet_log.range("C1").value = "Behavior"
    for key in serial_input_dict_copy.keys():
        #print(serial_input_dict_copy)
        if serial_input_dict_copy[key] != serial_input_dict[key]:
            cell_num = cell_num + 1
            sheet_log.range("A" + str(cell_num)).value = output_trial_num.get()
            sheet_log.range("B" + str(cell_num)).value = time.strftime('%H:%M:%S')
            sheet_log.range("C"+str(cell_num)).value = key

def final_data():
    #Store data at the end of each trail
    global final_data_num
    final_data_num = final_data_num + 1
    sheet_result.range("A1").value = "Date"
    sheet_result.range("A2").value = "Number"
    sheet_result.range("B2").value = "Duration(s)"
    sheet_result.range("C2").value = "Correct No."
    sheet_result.range("D2").value = "Incorrect No."
    sheet_result.range("E2").value = "Omission No."
    sheet_result.range("F2").value = "Total trials"
    sheet_result.range("A" + str(final_data_num)).value = trial_var.get()
    sheet_result.range("B" + str(final_data_num)).value = time_duration
    sheet_result.range("C" + str(final_data_num)).value = serial_input_dict['Correct']
    sheet_result.range("D" + str(final_data_num)).value = serial_input_dict['Incorrect']
    sheet_result.range("E" + str(final_data_num)).value = serial_input_dict['Omission']
    sheet_result.range("F" + str(final_data_num)).value = serial_input_dict['Total']


def data_save():
    output_path = output_file_dir.get()
    os.chdir(output_path)
    wb.save(time.strftime('%Y%m%d%H%M') + '.xlsx')
    #wb.close()

def button_close_fun():
    wb.close()
    root.destroy()

def button_select_path_fun():
    global path_var
    path_target = askdirectory()
    path_var.set(path_target)


#Parameters label
label_correct = tk.Label(root,text="Correct:", width = 10,anchor = 'e', font = ("Arial",20)).place(x=120, y=20)
label_wrong = tk.Label(root,text="Incorrect:", width = 10, anchor = 'e', font = ("Arial",20)).place(x=120, y=50)
label_ommision = tk.Label(root,text="Omissions:", width = 10, anchor = 'e', font = ("Arial",20)).place(x=120, y=80)
label_total = tk.Label(root,text="Trials:", width = 10,anchor = 'e', font = ("Arial",20)).place(x=120, y=110)

#Trial number label
label_trial_num = tk.Label(root, text="No.", width=40, anchor = 'w', font=("Arial", 20)).place(x=400, y=80)
label_output = tk.Label(root, text="Exp No.", width=9, anchor='w', font=("Arial", 10)).place(x=55, y=260)

#Trial number input
trial_var = tk.StringVar()
output_trial_num = tk.Entry(root,textvariable=trial_var,width = 20)
output_trial_num.place(x=150,y=260)

#Port number input
label_port = tk.Label(root, text="Port:", width=9, anchor='w', font=("Arial", 10)).place(x=350, y=260)
port_var = tk.StringVar()

#Setup default port number
port_var.set("3") 
port_entry = tk.Entry(root,textvariable=port_var,width = 10)
port_entry.place(x=400,y=260)

#Setup trial duration
label_time_limit = tk.Label(root, text="Duration(s):", width=14, anchor='w', font=("Arial", 10)).place(x=530, y=260)
time_limit_var = tk.StringVar()
entry_time_limit = tk.Entry(root,textvariable=time_limit_var,width = 5)
entry_time_limit.place(x=650,y=260)

#Time display
label_currenttime = tk.Label(root, text="Current time：", width=40, anchor = 'w', font=("Arial", 20)).place(x=400, y=20)
label_trailtime = tk.Label(root, text="Trial time：", width=40, anchor='w', font=("Arial", 20)).place(x=400, y=50)

#Start, stop and reset of the trail
button_start = tk.Button(root,text="Start",height = 2, width = 15, font=("Arial", 12), command = button_start_fun,state = 'normal').place(x=150, y=200)
button_stop = tk.Button(root,text="Stop",height = 2, width = 15, font=("Arial", 12), command = button_stop_fun,  state = 'disabled').place(x=350, y=200)
button_clear = tk.Button(root,text="Reset",height = 2, width = 15, font=("Arial", 12), command = button_clear_fun,).place(x=550, y=200)

#File output
label_output = tk.Label(root, text="File location:", width=13, anchor='w', font=("Arial", 10)).place(x=35, y=300)
button_output = tk.Button(root,text="Save",height = 1, width = 10, font=("Arial", 12), command = data_save, activeforeground = 'Grey').place(x=260, y=330)
button_select_path = tk.Button(root,text="...",height = 1, width = 3, font=("Arial", 12), command = button_select_path_fun).place(x=690, y=300)

path_var = tk.StringVar()
output_file_dir = tk.Entry(root,textvariable = path_var,width = 75)
output_file_dir.place(x=150,y=300)


button_close = tk.Button(root,text="Close",height = 1, width = 10, font=("Arial", 12), command = button_close_fun, activeforeground = 'Grey').place(x=400, y=330)


refresh()
root.mainloop()


