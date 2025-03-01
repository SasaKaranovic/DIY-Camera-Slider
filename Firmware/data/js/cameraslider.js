// Shorthand for $( document ).ready()
var tooltipTriggerList = [].slice.call(document.querySelectorAll('[data-bs-toggle="tooltip"]'))
var tooltipList = tooltipTriggerList.map(function (tooltipTriggerEl) {
  return new bootstrap.Tooltip(tooltipTriggerEl, {
    'custom-class': 'custom-tooltip'
  })
})
$(function() {
    $('[data-bs-toggle="tooltip"]').tooltip();
	
	$('#sliderStartPosition').on('input change', function() {
		const label = $("label[for='labelStartPosition']");
		let span = label.find("span.dynamic-text");
	
		if (span.length === 0) {
			span = $("<span class='dynamic-text'></span>");
			label.append(span);
		}
	
		span.text(parseInt(this.value) + "mm");
	});

	$('#sliderEndPosition').on('input change', function() {
		const label = $("label[for='labelEndPosition']");
		let span = label.find("span.dynamic-text");
	
		if (span.length === 0) {
			span = $("<span class='dynamic-text'></span>");
			label.append(span);
		}
	
		span.text(parseInt(this.value) + "mm");
	});

	$('#sliderRotateBy').on('input change', function() {
		const label = $("label[for='labelRotateBy']");
		let span = label.find("span.dynamic-text");
	
		if (span.length === 0) {
			span = $("<span class='dynamic-text'></span>");
			label.append(span);
		}
	
		span.text(parseInt(this.value) + "°");
	});


	$('#sliderManualPosition').on('input change', function(){	
		ui_update_slider_label();
	});

	$('#sliderManualRotation').on('input change', function(){
        ui_update_rotation_label();

	});

    $("#sliderManualRotation").dblclick(function() {
      this.value = 0;
      ui_update_rotation_label();
    });

    $("label[for='labelManualRotation']").dblclick(function() {
      $("#sliderManualRotation").val(0);
      ui_update_rotation_label();
    });

    $('#sliderManualPosition').bind('mousewheel', function (event) {
        mousewheel_slide(event);
    });

    $("label[for='labelManualPosition']").bind('mousewheel', function (event) {
        mousewheel_slide(event);
    });

    $('#sliderManualRotation').bind('mousewheel', function (event) {
        mousewheel_rotation(event);
    });

    $("label[for='labelManualRotation']").bind('mousewheel', function (event) {
        mousewheel_rotation(event);
    });


	$('#checkFromStartToEnd').change(function() {
		if( $(this).prop("checked") ){
			$("#manualSlidingPos").hide();
		}
		else{
			$("#manualSlidingPos").show();
		}
	});


	$('#refreshStatus').click(function(){
		readStatus();
	});
	


	$('#btnManualMove').on('click', function(){
		var posSlide = $("#sliderManualPosition").val();
		var cfgSliderSpeed = $("#cfgSliderSpeed").val();
		var cfgSliderAccel = $("#cfgSliderAccel").val();

		var posRotate = $("#sliderManualRotation").val();
		var cfgRotationSpeed = $("#cfgRotationSpeed").val();
		var cfgRotationAccel = $("#cfgRotationAccel").val();

		console.log("posSlide: " + posSlide);
		console.log("cfgSliderSpeed: " + cfgSliderSpeed);
		console.log("cfgSliderAccel: " + cfgSliderAccel);

		console.log("posRotate: " + posRotate);
		console.log("cfgRotationSpeed: " + cfgRotationSpeed);
		console.log("cfgRotationAccel: " + cfgRotationAccel);

		moveToPosition(posSlide, cfgSliderSpeed, cfgSliderAccel, posRotate, cfgRotationSpeed, cfgRotationAccel)
	});

	// OnClick for Auto Move button
	$('#btnAutoSlide').on('click', function(){
		var bStartStopSlide = $('#checkFromStartToEnd').prop("checked");
		console.log(bStartStopSlide);

		var hours 		= parseInt($("#inputHours").val());
		var minutes 	= parseInt($("#inputMinutes").val());
		var seconds 	= parseInt($("#inputSeconds").val());

		if( !$.isNumeric(hours) ) 	{ hours = 0; }
		if( !$.isNumeric(minutes) ) 	{ minutes = 0; }
		if( !$.isNumeric(seconds) ) 	{ seconds = 0; }

		var argSeconds 	= parseInt(seconds) + parseInt(minutes)*60 + parseInt(hours)*3600;

		console.log("hours: " + hours);
		console.log("minutes: " + minutes);
		console.log("seconds: " + seconds);
		console.log("argSeconds: " + argSeconds);


		// Slide from Start-to-Stop position
		if(bStartStopSlide == true)
		{
			$.ajax({
			  url: "/api/move-start-to-stop",
			  type: "get", //send it through get method
			  data: { 
			    seconds: argSeconds,
			  },
			  success: function(response) {
			    //Do Something
			    console.log(response);
			  },
			  error: function(xhr) {
			    //Do Something to handle error
			  }
			});
		}
		// Othwerise slide from manually specified start-stop
		else
		{
			var startPos 	= $("#sliderStartPosition").val();
			var endPos 		= $("#sliderEndPosition").val();
			var rotateBy 	= $("#sliderRotateBy").val();
			$.ajax({
			  url: "/api/move-start-to-stop",
			  type: "get", //send it through get method
			  data: { 
			    startPos: startPos,
			    endPos: endPos,
			    rotateBy: rotateBy,
			    seconds: argSeconds,
			  },
			  success: function(response) {
			    //Do Something
			    console.log(response);
			  },
			  error: function(xhr) {
			    //Do Something to handle error
			  }
			});


			console.log("startPos: " + startPos);
			console.log("endPos: " + endPos);
			console.log("rotateBy: " + rotateBy);
		}

	});

	$('#btnHomeSlider').on('click', function(){
		// Send 
		$.ajax({
		  url: "/api/home-slider",
		  type: "get", //send it through get method
		  success: function(response) {
		    //Do Something
		    console.log(response);
		  },
		  error: function(xhr) {
		    //Do Something to handle error
		  }
		});
	});
	
	$('#btnHomeRotation').on('click', function(){
		// Send 
		$.ajax({
		  url: "/api/home-rotation",
		  type: "get", //send it through get method
		  success: function(response) {
		    //Do Something
		    console.log(response);
		  },
		  error: function(xhr) {
		    //Do Something to handle error
		  }
		});
	});

	$('#btnMotorsOff').on('click', function(){
		// Send 
		$.ajax({
		  url: "/api/motors-turn-off",
		  type: "get", //send it through get method
		  success: function(response) {
		    //Do Something
		    console.log(response);
		  },
		  error: function(xhr) {
		    //Do Something to handle error
		  }
		});
	});

	$('#btnMotorsOn').on('click', function(){
		// Send 
		$.ajax({
		  url: "/api/motors-turn-on",
		  type: "get", //send it through get method
		  success: function(response) {
		    //Do Something
		    console.log(response);
		  },
		  error: function(xhr) {
		    //Do Something to handle error
		  }
		});
	});

	$('#btnSetStartPos').on('click', function(){
		// Send 
		$.ajax({
		  url: "/api/position-save-start",
		  type: "get", //send it through get method
		  success: function(response) {
		    //Do Something
		    $('#btnSetStartPos').removeClass('btn-primary');
		    $('#btnSetStartPos').addClass('btn-success');
		    console.log(response);
		  },
		  error: function(xhr) {
		    //Do Something to handle error
		    console.log(xhr);
		  }
		});
	});

	$('#btnSetEndPos').on('click', function(){
		// Send 
		$.ajax({
		  url: "/api/position-save-end",
		  type: "get", //send it through get method
		  success: function(response) {
		    //Do Something
		    $('#btnSetEndPos').removeClass('btn-primary');
		    $('#btnSetEndPos').addClass('btn-success');
		    console.log(response);
		  },
		  error: function(xhr) {
		    //Do Something to handle error
		    console.log(xhr);
		  }
		});
	});

	$('#btnMoveToStart').on('click', function(){
        var xSpeed = $("#cfgSliderSpeed").val();
        var xAccel = $("#cfgSliderAccel").val();
        var rSpeed = $("#cfgRotationSpeed").val();
        var rAccel = $("#cfgRotationAccel").val();

		// Send 
		$.ajax({
		  url: "/api/position-goto-start",
		  type: "get", //send it through get method
          data: {
            xSpeed: xSpeed,
            xAccel: xAccel,
            rSpeed: rSpeed,
            rAccel: rAccel,
          },
		  success: function(response) {
		    //Do Something
		    $('#btnSetEndPos').removeClass('btn-primary');
		    $('#btnSetEndPos').addClass('btn-success');
		    console.log(response);
		  },
		  error: function(xhr) {
		    //Do Something to handle error
		    console.log(xhr);
		  }
		});
	});

	$('#btnMoveToEnd').on('click', function(){
        var xSpeed = $("#cfgSliderSpeed").val();
        var xAccel = $("#cfgSliderAccel").val();
        var rSpeed = $("#cfgRotationSpeed").val();
        var rAccel = $("#cfgRotationAccel").val();

		// Send 
		$.ajax({
		  url: "/api/position-goto-end",
		  type: "get", //send it through get method
          data: {
            xSpeed: xSpeed,
            xAccel: xAccel,
            rSpeed: rSpeed,
            rAccel: rAccel,
          },
		  success: function(response) {
		    //Do Something
		    $('#btnSetEndPos').removeClass('btn-primary');
		    $('#btnSetEndPos').addClass('btn-success');
		    console.log(response);
		  },
		  error: function(xhr) {
		    //Do Something to handle error
		    console.log(xhr);
		  }
		});
	});


    $('#btn_ResetSettings').on('click', function(){
        // Send
        $.ajax({
          url: "/api/settings-reset",
          type: "get", //send it through get method
          success: function(response) {
            //Do Something
            console.log(response);
          },
          error: function(xhr) {
            //Do Something to handle error
            console.log(xhr);
          }
        });
    });


    $('#btn_SaveSettings').on('click', function(){
        var config_rail_length = $("#config_rail_length").val();
        var slider_min_step = $("#config_rail_min_step").val();
        var homing_speed_slider = $("#config_homing_speed_slider").val();
        var homing_speed_rotation = $("#config_homing_speed_rotation").val();
        var steps_per_mm_slider = $("#config_steps_per_mm_slider").val();
        var steps_per_mm_rotation = $("#config_steps_per_mm_rotation").val();
        var invert_homing_direction = $("#invert_homing_direction").is(":checked")
        var invert_slider_direction = $("#invert_slider_direction").is(":checked")
        var invert_rotation_direction = $("#invert_rotation_direction").is(":checked")

        if(invert_homing_direction == true)
        {
            invert_homing_direction = -1;
        }
        else
        {
            invert_homing_direction = 1;
        }

        if(invert_slider_direction == true)
        {
            invert_slider_direction = -1;
        }
        else
        {
            invert_slider_direction = 1;
        }

        if(invert_rotation_direction == true)
        {
            invert_rotation_direction = -1;
        }
        else
        {
            invert_rotation_direction = 1;
        }

       update_settings('set_rail_length', config_rail_length);
       update_settings('set_slider_min_step', slider_min_step);
       update_settings('set_homing_speed_slide', homing_speed_slider);
       update_settings('set_homing_speed_pan', homing_speed_rotation);
       update_settings('set_steps_per_mm_slider', steps_per_mm_slider);
       update_settings('set_steps_per_deg', steps_per_mm_rotation);
       update_settings('set_homing_direction', invert_homing_direction);
       update_settings('set_slider_direction', invert_slider_direction);
       update_settings('set_pan_direction', invert_rotation_direction);
    });



	// Finally update status
	readStatus();
});

function mousewheel_slide(event){
    event.preventDefault();
    console.log(event.originalEvent);

    current_value = parseInt($('#sliderManualPosition').val());
    slider_step = 10;

    if (event.originalEvent.ctrlKey){
        slider_step = 50;
    }
    else if (event.originalEvent.altKey){
        slider_step = 1;
    }

    wd = event.originalEvent.wheelDelta;
    if (wd > 0 ){
        new_value = current_value + slider_step;
    }
    else{
        new_value = current_value - slider_step;
    }

    $('#sliderManualPosition').prop('value', new_value);
    ui_update_slider_label();
}

function mousewheel_rotation(event){
    event.preventDefault();

    current_value = parseInt($('#sliderManualRotation').val());
    rotation_step = 1;

    if (event.originalEvent.ctrlKey){
        rotation_step = 5;
    }

    wd = event.originalEvent.wheelDelta;
    if (wd > 0 ){
        new_value = current_value + rotation_step;
    }
    else{
        new_value = current_value - rotation_step;
    }

    $('#sliderManualRotation').prop('value', new_value);
    ui_update_rotation_label();
}

function ui_update_slider_label() {
    const label = $("label[for='labelManualPosition']");
    let span = label.find("span.dynamic-text");

    if (span.length === 0) {
        span = $("<span class='dynamic-text'></span>");
        label.append(span);
    }

    span.text(parseInt($('#sliderManualPosition').val()) + "mm");
}

function ui_update_rotation_label() {
    const label = $("label[for='labelManualRotation']");
    let span = label.find("span.dynamic-text");

    if (span.length === 0) {
        span = $("<span class='dynamic-text'></span>");
        label.append(span);
    }

    span.text(parseInt($('#sliderManualRotation').val()) + "°");
}

function moveToPosition(xPos, xSpeed, xAccel, rPos, rSpeed, rAccel){
	// Send 
	$.ajax({
	  url: "/api/move-to-position",
	  type: "get", //send it through get method
	  data: { 
	    xPos: xPos, 
	    xSpeed: xSpeed, 
	    xAccel: xAccel,
	    rPos: rPos,
	    rSpeed: rSpeed,
	    rAccel: rAccel,
	  },
	  success: function(response) {
	    //Do Something
	    console.log(response);
	  },
	  error: function(xhr) {
	    //Do Something to handle error
	  }
	});
}


function readStatus(){

	// Send 
	$.ajax({
	  url: "/api/camera-slider-status",
	  type: "get", //send it through get method
	  success: function(response) {
	    console.log(response);

	    var jsonResponse = jQuery.parseJSON(response);

	    // Current position and rotation
	    $('#status-position').text(jsonResponse.posX);
	    $('#status-rotation').text(jsonResponse.posZ);

	    // Start position and rotation
	    $('#status-startSlide').text(jsonResponse.spX);
	    $('#status-startRotation').text(jsonResponse.spZ);

	    // End position and rotation
	    $('#status-endSlide').text(jsonResponse.epX);
	    $('#status-endRotation').text(jsonResponse.epZ);


	    // Slider state
	    // $('#status-state').text(sliderStates[jsonResponse.state]);


	    // Homed status
	    if(jsonResponse.homed == 1)
	    {
	    	// Homed
	    	$('#status-homed').text('Homed and ready');
	    	$('#status-homed').addClass('badge-success');
	    	$('#status-homed').removeClass('badge-warning');
	    }
	    else
	    {
	    	// Waiting for home
	    	$('#status-homed').text('Home first');
	    	$('#status-homed').removeClass('badge-success');
	    	$('#status-homed').addClass('badge-warning');
	    }

	  },
	  error: function(xhr) {
	    //Do Something to handle error
	  }
	});
}

function update_settings(parameter, value){
    console.log("Parameter '"+ parameter +"' ("+ value +")");

    value = parseInt(value)

    if (isNaN(value))
    {
        console.log("Parameter for '"+ parameter +"' ("+ value +") is not valid numeric value.");
        return;
    }


    // Send
    $.ajax({
      url: update_settings_urls[parameter],
      type: "get", //send it through get method
      data: {
        value: value,
      },
      success: function(response) {
        //Do Something
        console.log(response);
      },
      error: function(xhr) {
        //Do Something to handle error
      }
    });
}

var update_settings_urls ={
    'set_homing_speed_slide' : '/api/set-homing-speed-slide',
    'set_homing_speed_pan' : '/api/set-homing-speed-pan',
    'set_steps_per_mm_slider' : '/api/set-steps-per-mm',
    'set_steps_per_deg' : '/api/set-steps-per-deg',
    'set_homing_direction' : '/api/set-homing-direction',
    'set_slider_direction' : '/api/set-slide-direction',
    'set_pan_direction' : '/api/set-pan-direction',
    'set_rail_length' : '/api/set-rail-length',
    'set_slider_min_step': '/api/set-slider-min-step'
}



var sliderStates = 
[
	'SLIDER_MOTORS_OFF', 
    'SLIDER_IDLE', 
    'SLIDER_HOMING',
    'SLIDER_MOVING_TO_START',
    'SLIDER_MOVING_TO_END',
    'SLIDER_READY',
    'SLIDER_WORKING',
    'SLIDER_LAST'
];
