import tkinter as tk
from tkinter import ttk
import mido
import math

class SPDIFMixer:
    def __init__(self, root):
        self.root = root
        self.root.title("SPDIF Mixer")
        self.root.geometry("450x550")
        self.root.configure(bg="#494949")
        
        # Variables MIDI
        self.midi_output = None
        self.midi_channel = 0
        
        # Configuration des sliders
        self.slider_config = [
            {"name": "SPDIF IN 1", "cc": 20, "color": "#4a9eff"},
            {"name": "SPDIF IN 2", "cc": 21, "color": "#ff6b4a"},
            {"name": "SPDIF IN 3", "cc": 22, "color": "#4aff6b"},
            {"name": "MASTER OUT", "cc": 23, "color": "#ffaa4a"}
        ]
        
        self.sliders = []
        self.value_entries = []
        self.updating_from_entry = False  # Flag pour éviter les boucles
        
        self.setup_ui()
        self.refresh_midi_ports()
        
    def db_to_midi(self, db):
        """Convertit dB (-45 à +6) en valeur MIDI (0-127)"""
        # Mapping linéaire: -45dB = 0, +6dB = 127
        normalized = (db + 45.0) / 51.0  # 51 = plage totale (6 - (-45))
        midi_value = int(normalized * 127)
        return max(0, min(127, midi_value))
    
    def midi_to_db(self, midi_value):
        """Convertit valeur MIDI (0-127) en dB (-45 à +6)"""
        normalized = midi_value / 127.0
        db = -45.0 + (normalized * 51.0)
        return db
    
    def db_to_gain(self, db):
        """Convertit dB en gain linéaire pour affichage"""
        return math.pow(10.0, db / 20.0)
    
    def setup_ui(self):
        # Frame supérieur pour la configuration MIDI
        config_frame = tk.Frame(self.root, bg="#666666")
        config_frame.pack(pady=10, padx=10, fill='x')
        
        # Sélection interface MIDI
        tk.Label(config_frame, text="Interface MIDI:", bg="#666666", fg='white').grid(row=0, column=0, padx=5, sticky='w')
        self.midi_port_var = tk.StringVar()
        self.midi_port_combo = ttk.Combobox(config_frame, textvariable=self.midi_port_var, width=30, state='readonly')
        self.midi_port_combo.grid(row=0, column=1, padx=5)
        self.midi_port_combo.bind('<<ComboboxSelected>>', self.on_midi_port_change)
        
        # Bouton refresh
        tk.Button(config_frame, text="↻", command=self.refresh_midi_ports, bg='#444', fg='white', width=3).grid(row=0, column=2, padx=5)
        
        # Sélection canal MIDI
        tk.Label(config_frame, text="Canal MIDI:", bg='#666666', fg='white').grid(row=1, column=0, padx=5, pady=5, sticky='w')
        self.channel_var = tk.IntVar(value=1)
        channel_combo = ttk.Combobox(config_frame, textvariable=self.channel_var, width=10, state='readonly')
        channel_combo['values'] = list(range(1, 17))
        channel_combo.grid(row=1, column=1, padx=5, sticky='w')
        channel_combo.bind('<<ComboboxSelected>>', self.on_channel_change)
        
        # Frame pour les sliders
        sliders_frame = tk.Frame(self.root, bg='#2b2b2b')
        sliders_frame.pack(pady=20, expand=True, fill='both')
        
        # Créer les 4 sliders
        for i, config in enumerate(self.slider_config):
            self.create_slider(sliders_frame, i, config)
    
    def create_slider(self, parent, index, config):
        # Frame pour chaque slider
        frame = tk.Frame(parent, bg='#2b2b2b')
        frame.grid(row=0, column=index, padx=15, pady=10)
        
        # Label du nom
        tk.Label(frame, text=config["name"], bg='#2b2b2b', fg='white', font=('Arial', 10, 'bold')).pack()
        
        # Entry éditable pour la valeur en dB
        entry_var = tk.StringVar(value="0.0")
        value_entry = tk.Entry(
            frame, 
            textvariable=entry_var,
            bg='#333', 
            fg=config["color"], 
            font=('Arial', 12, 'bold'),
            justify='center',
            width=8,
            relief='flat',
            highlightthickness=1,
            highlightbackground='#555',
            highlightcolor=config["color"]
        )
        value_entry.pack(pady=5)
        value_entry.bind('<Return>', lambda e, idx=index: self.on_entry_change(idx))
        value_entry.bind('<FocusOut>', lambda e, idx=index: self.on_entry_change(idx))
        self.value_entries.append(entry_var)
        
        # Label "dB"
        tk.Label(frame, text="dB", bg='#2b2b2b', fg='#888', font=('Arial', 9)).pack()
        
        # Slider vertical
        slider = tk.Scale(
            frame,
            from_=6,
            to=-45,
            resolution=0.1,
            orient='vertical',
            length=300,
            bg='#444',
            fg='white',
            troughcolor='#333',
            activebackground=config["color"],
            highlightthickness=0,
            showvalue=False,  # Cacher la valeur native du slider
            command=lambda val, idx=index: self.on_slider_change(idx, float(val))
        )
        slider.set(0)  # Position par défaut à 0dB
        slider.pack(pady=10)
        self.sliders.append(slider)
        
        # Label CC MIDI
        tk.Label(frame, text=f"CC {config['cc']}", bg='#2b2b2b', fg='#888', font=('Arial', 8)).pack()
    
    def on_slider_change(self, index, db_value):
        if self.updating_from_entry:
            return
            
        # Mettre à jour l'affichage de l'entry
        self.value_entries[index].set(f"{db_value:+.1f}")
        
        # Envoyer MIDI
        self.send_midi(index, db_value)
    
    def on_entry_change(self, index):
        """Appelé quand l'utilisateur édite manuellement la valeur"""
        try:
            # Récupérer et valider la valeur
            value_str = self.value_entries[index].get().strip()
            db_value = float(value_str)
            
            # Limiter à la plage -45 à +6
            db_value = max(-45.0, min(6.0, db_value))
            
            # Mettre à jour le slider
            self.updating_from_entry = True
            self.sliders[index].set(db_value)
            self.updating_from_entry = False
            
            # Reformater l'affichage
            self.value_entries[index].set(f"{db_value:+.1f}")
            
            # Envoyer MIDI
            self.send_midi(index, db_value)
            
        except ValueError:
            # En cas d'erreur, restaurer la valeur du slider
            current_value = self.sliders[index].get()
            self.value_entries[index].set(f"{current_value:+.1f}")
    
    def send_midi(self, index, db_value):
        """Envoie le message MIDI CC"""
        cc_num = self.slider_config[index]["cc"]
        if cc_num is not None and self.midi_output is not None:
            midi_value = self.db_to_midi(db_value)
            try:
                msg = mido.Message('control_change', 
                                 channel=self.midi_channel, 
                                 control=cc_num, 
                                 value=midi_value)
                self.midi_output.send(msg)
            except Exception as e:
                print(f"Erreur MIDI: {e}")
    
    def refresh_midi_ports(self):
        """Rafraîchit la liste des ports MIDI disponibles"""
        ports = mido.get_output_names()
        self.midi_port_combo['values'] = ports
        if ports:
            self.midi_port_combo.current(0)
            self.on_midi_port_change(None)
    
    def on_midi_port_change(self, event):
        """Change le port MIDI sélectionné"""
        if self.midi_output:
            self.midi_output.close()
        
        port_name = self.midi_port_var.get()
        if port_name:
            try:
                self.midi_output = mido.open_output(port_name)
                print(f"Connecté à: {port_name}")
            except Exception as e:
                print(f"Erreur de connexion MIDI: {e}")
                self.midi_output = None
    
    def on_channel_change(self, event):
        """Change le canal MIDI (0-15 en interne, affiché 1-16)"""
        self.midi_channel = self.channel_var.get() - 1
        print(f"Canal MIDI: {self.channel_var.get()}")

if __name__ == "__main__":
    root = tk.Tk()
    app = SPDIFMixer(root)
    root.mainloop()