import mraa
import time
import requests
import json
import pyupm_tsl2561 as upmTsl2561
import pyupm_grovemoisture as upmMoisture
from ISStreamer.Streamer import Streamer


streamer = Streamer(
    bucket_key='Hackathons',
    bucket_name='WaterCloud',
    access_key='2RGoVkZ7UJsftizkKhXapKHzoaaAI5wK'
)

wunderground_api_key = '8c75b3edbac674fc'
wunderground_url = 'api.wunderground.com/api/{api}/forecast/q/CO/Arvada.json'\
    .format(api=wunderground_api_key)

yesterday = 'api.wunderground.com/api/8c75b3edbac674fc/yesterday/q/' \
            'CO/Arvada.json'

led = mraa.Gpio(13)
led.dir(mraa.DIR_OUT)
led.write(1)
time.sleep(0.25)
led.write(0)


myMoisture = upmMoisture.GroveMoisture(1)

relay = mraa.Gpio(2)
relay.dir(mraa.DIR_OUT)

rain_override = upmTsl2561.TSL2561()

temp_override = mraa.Aio(0)


while 1:
    # default score
    water_lawn = 50

    # get weather data from weather underground
    r = requests.get('http://{url}'.format(url=wunderground_url))
    payload = None
    if r.text:
        payload = json.loads(r.text)
    if not payload:
        time.sleep(2)
        continue

    today_forecast = payload['forecast']['simpleforecast']['forecastday'][0]
    # friendly output for LCD screen perhaps
    max_temp = '{f}F/{c}C'.format(
        f=today_forecast['high']['fahrenheit'],
        c=today_forecast['high']['celsius'],
    )
    # prob of precip
    pop = today_forecast['pop']
    # if we do get rain, how many inches
    inches = today_forecast['qpf_allday']['in']

    # OVERRIDES
    # I'm using light sensors to mimic high rain or low/high temp
    rain_override_val = rain_override.getLux()
    temp_override_val = temp_override.readFloat() * 200
    if rain_override_val < 25:
        water_lawn = -1000
    if temp_override_val < 10:
        today_forecast['high']['fahrenheit'] = 100
        water_lawn = 1000
    elif temp_override_val < 25:
        today_forecast['low']['fahrenheit'] = 32
        water_lawn = -1000

    # if moisture_sensor <= 50:
    #     water_lawn = True
    moisture_reading = myMoisture.value()
    if 0 <= moisture_reading < 300:
        # pretty dry
        water_lawn += 50
    elif 300 <= moisture_reading < 350:
        # moisture is moderate
        water_lawn -= 25
    elif moisture_reading >= 350:
        # saturation
        water_lawn -= 100

    # adjust water score based on prob of precip
    # if pop > 50%, more likely it will rain, so we subtract points
    # if pop < 50%, less likely it will rain, so we add points
    water_lawn += 50 - pop

    # if we're going to get rain, let's work on some water points based on
    # expected rainfall
    if inches < 0.05:
        # not a lot of rainfall, so let's add more points
        water_lawn += 50
    elif inches < 0.25:
        # fair amount for a city zone, so let's add fewer points
        # in case pop is low
        water_lawn += 10
    elif inches >= 0.5:
        # more than half an inch in a city is plenty to keep our lawn wet
        water_lawn -= 100

    if today_forecast['low']['fahrenheit'] <= 35:
        # send email alert
        import sendgrid

        sg_api = 'SG.8bDRUZ9nQUWfbA0IvitwtQ.5AMlH9B6-' \
                 'S8nFmviuBP6evR93m4uheExjKikE6yyoYE'

        sg = sendgrid.SendGridClient(sg_api)

        message_text = "There is a potential freeze condition in your area, " \
                       "and sprinkler systems have been halted. If this pers" \
                       "ists, you may want to consider clearing the water fr" \
                       "om your sprinkler systems to avoid frozen pipes lead" \
                       "ing to cracks and leaks."
        message = sendgrid.Mail()
        message.add_to('Ian Douglas <ian.douglas@iandouglas.com>')
        message.set_subject('Potential Frost/Freeze condition!')
        message.set_from('Ian Douglas <ian.douglas@iandouglas.com>')
        message.set_html(message_text)
        message.set_text(message_text)
        status, msg = sg.send(message)

        water_lawn -= 100
    elif today_forecast['high']['fahrenheit'] <= 65:
        # evaporation is a much lower concern here
        water_lawn -= 25
    elif today_forecast['high']['fahrenheit'] >= 85:
        # evaporation is a high concern, so let's water the lawn
        water_lawn += 75

    print('-' * 100)

    print('moisture: {m}'.format(m=moisture_reading))
    streamer.log("Moisture", moisture_reading)

    print('water score: {water}'.format(water=water_lawn))
    streamer.log("WaterScore", water_lawn)

    print('temp_or_val: {temp}'.format(temp=temp_override_val))
    streamer.log("TempOverideScore", temp_override_val)

    print('rain_or_val: {temp}'.format(temp=rain_override_val))
    streamer.log("RainOverideScore", rain_override_val)

    print('hi temp: {temp}'.format(temp=today_forecast['high']['fahrenheit']))
    streamer.log("HighTempForecast", today_forecast['high']['fahrenheit'])

    print('lo temp: {temp}'.format(temp=today_forecast['low']['fahrenheit']))
    streamer.log("LowTempForecast", today_forecast['low']['fahrenheit'])

    print('temp: {temp}'.format(temp=max_temp))

    print('pop: {pop}'.format(pop=pop))
    streamer.log("ProbOfPrecip", pop)

    print('inches: {inches}'.format(inches=inches))
    streamer.log("RainfallForecast", inches)

    if water_lawn >= 150:
        # click the relay on/off so we know we should be watering the lawn
        relay.write(1)
        led.write(1)
        time.sleep(0.5)
        relay.write(0)
        led.write(0)
        time.sleep(0.5)

    time.sleep(8)
