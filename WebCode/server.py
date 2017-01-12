#!flask/bin/python -tt
import os, serial, json, pygal
from flask import Flask, render_template, request
from wtforms import Form, TextField, TextAreaField,IntegerField, validators, StringField, SubmitField

app = Flask(__name__)
app.config.from_object(__name__)
app.config['SECRET_KEY'] = 'I am not telling you the key'

class sweapForm(Form):
	startFreq = IntegerField('StartFrequency', validators=[validators.required()])
	stopFreq = TextField('StopFrequency', validators=[validators.required()])
	numOfSamples = TextField('NumOfSamples', validators=[validators.required()])

	def validate(self):
		if not super(sweapForm, self).validate():
			return False
		if not self.startFreq.data and not self.stopFreq.data and not self.numOfSamples.data:
			msg = 'All fields need to be filled in'
			self.startFreq.errors.append(msg)
			self.stopFreq.errors.append(msg)
			self.numOfSamples.errors.append(msg)
		return True;

@app.route('/')
def mainIndex():
	return render_template('index.html')

@app.route('/examples')
def examples():
	return render_template('examples.html')

@app.route('/aa-sweap', methods=['GET', 'POST'])
def aasweap():
	print "Hun??? : "+request.method
	form = sweapForm(request.form)

	if request.method == 'POST':
		if form.validate():
			startFrequency = request.form['startFreq']
			stopFrequency = request.form['stopFreq']
			numOfSamples =request.form['numOfSamples']
			aaPort = "/dev/cu.usbmodem1411"
			# Connect with arduino
			serialPort = serial.Serial(aaPort, 9600, timeout = 10)
			#
			# Command Format is the Command followed by data
			# Usually the Data is a number and multiple commands can be on one line
			# X99999 Example command
			# Commands
			# A - Set the Starting Frequency
			# B - Set the ending Frequency
			# N - Set the Number of Samples Between the two
			# Tell arduino that those Following numbers are for the starting frequency
			strTmp = "A"+str(startFrequency)+"B"+str(stopFrequency)+"N"+str(numOfSamples)
			print strTmp
			serialPort.write(strTmp)
			serialPort.write("\n")
			serialPort.flush()
			#
			# SEND SWEEP Commands
			#
			serialPort.write("S\n")
			inputLine  = ""
			inputBuffer = ""
			while (inputLine.rfind(']') == -1) :
				inputLine = serialPort.readline()
				print(inputLine)
				inputBuffer = inputBuffer + inputLine


			parsed = json.loads(inputBuffer)
			print(parsed)
			vswr = [float(i['VSWR']) for i in parsed]
			frequency = [float(i['CF'])/100000.0 for i in parsed]
			#    npArray = np.array(vswr)
			#    vswrDwnSample = decimate(npArray,10,axis=0)
			#    npArray = np.array(frequency)
			#    frequencyDwnSample = decimate(npArray,10,axis=0)
			print("Length of vswr : "+str(len(vswr)))
			title = 'Frequency Sweep for Start %i Finish %i' % (int(startFrequency), int(stopFrequency))
			#    lineGraph = pygal.Line(width=1200, height=600,
			#                                  explicit_size=True, title=title, style=DarkSolarizedStyle)
			lineGraph = pygal.Line(width=900, height=500,
                                  	explicit_size=True,
                                  	title=title,
                                  	disable_xml_declaration=True)
			#    lineGraph.x_labels = frequencyDwnSample.tolist()
			#    lineGraph.add('VSWR %', vswrDwnSample.tolist())
			lineGraph.x_labels = frequency
			lineGraph.add('VSWR %', vswr)


			print "Boo :"+startFrequency
			return render_template('aa-sweap.html', form=form,
						startFreq=startFrequency,
						stopFreq=stopFrequency,
						numSamp=numOfSamples,
						graph=lineGraph)
		else:
			print "Bad error "
			return render_template('aa-sweap.html', form=form)
	else:
		return render_template('aa-sweap.html', form=form)

# start the server
if __name__ == '__main__':
    app.run(host=os.getenv('IP', '0.0.0.0'), port =int(os.getenv('PORT', 8082)), debug=True)
