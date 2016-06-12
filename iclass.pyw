try:
    import tkinter as tk # Python 3
    from tkinter import messagebox as msgbox 
except:
    import Tkinter as tk # Python 2
    import tkMessageBox as msgbox
import pygubu
import subprocess
from pprint import pprint

startupinfo = subprocess.STARTUPINFO()
startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW

class Application(pygubu.TkApplication):
    def _create_ui(self):
        global builder
        #1: Create a builder
        self.builder = builder = pygubu.Builder()

        #2: Load an ui file
        builder.add_from_file('iclass.ui')

        #3: Create the widget using a master as parent
        self.mainwindow = builder.get_object('iclass', self.master)
        self.set_title('HID iClass Cloner')

        builder.connect_callbacks(self)

    def on_block_select(self, event):
        w = event.widget
        block = int(w.curselection()[0])
        data = w.get(block)
        print block, data
        self.load_data(block, data)

    def load_data(self, block, data):
        data_entry = self.builder.get_object("Data")
        data_entry.delete(0, tk.END)
        data_entry.insert(0, data)

        block_entry = self.builder.get_object("Block Choice")
        block_entry.delete(0, tk.END)
        block_entry.insert(0, block)

    def read_card(self):
        print "READCARD"
        self.block_list = block_list = self.builder.get_object("Block List")

        self.block_list.bind('<<ListboxSelect>>', self.on_block_select)
        self.block_list.delete(0, tk.END)

        self.block_list.bind('<Control-a>', lambda x: self.block_list.select_set(0,tk.END))

        try:
            blocks = subprocess.check_output(['iclass.exe', 'read'], startupinfo=startupinfo).split()
            for block in blocks:
                block_list.insert(tk.END, block)
        except Exception as e:
            status = e.output
            msgbox.showinfo('Result', status)


    def write_block(self):
        print "WRITEBLOCK"
        block_list = self.builder.get_object("Block List")

        block_entry = self.builder.get_object("Block Choice")
        block = block_entry.get()
        data_entry = self.builder.get_object("Data")
        data = data_entry.get()
        try:
            status = subprocess.check_output(['iclass.exe', 'write' , block, data], startupinfo=startupinfo)
            check = subprocess.check_output(['iclass.exe', 'read', block], startupinfo=startupinfo)
            print repr(check), repr(data)
            if check.strip() != data.strip():
                msgbox.showinfo('Result', "FAIL: Data read does not match data written")
            else:
                msgbox.showinfo('Result', status)
        except Exception as e:
            status = e.output

        block_list.delete(block)
        block_list.insert(block, data)
        


if __name__ == '__main__':
    root = tk.Tk()
    root.resizable(0, 0)
    app = Application(root)
    app.run()