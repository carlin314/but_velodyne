/*
 * sample_hue_detector_ros.cpp
 *
 *  Created on: 12.7.2013
 *      Author: zdenal
 */

#include "rt_road_detection/detectors/sample_hue_detector_ros.h"


using namespace rt_road_detection;

SampleHueDetectorRos::SampleHueDetectorRos(ros::NodeHandle private_nh) {

	nh_ = private_nh;

	it_.reset(new image_transport::ImageTransport(nh_));

	int hue_min,hue_max,median_ks;

	nh_.param("hue_min",hue_min,30);
	nh_.param("hue_max",hue_max,70);
	nh_.param("median_ks",median_ks,7);

	det_.reset(new SampleHueDetector(hue_min,hue_max,median_ks));

	std::string top_rgb_in = "rgb_in";
	std::string top_det_out = "det_out";

	if (top_rgb_in == ros::names::remap(top_rgb_in)) ROS_WARN("Topic %s was not remapped!",top_rgb_in.c_str());
	else ROS_INFO("Topic %s remapped to %s.",top_rgb_in.c_str(),ros::names::remap(top_rgb_in).c_str());

	if (top_det_out == ros::names::remap(top_det_out)) ROS_WARN("Topic %s was not remapped!",top_det_out.c_str());
	else ROS_INFO("Topic %s remapped to %s.",top_det_out.c_str(),ros::names::remap(top_det_out).c_str());

	//image_transport::TransportHints hints("raw", ros::TransportHints(), getPrivateNodeHandle());
	sub_ = it_->subscribe(ros::names::remap(top_rgb_in), 1, &SampleHueDetectorRos::imageCallback,this);
	pub_ = it_->advertise(ros::names::remap(top_det_out), 1);

	dyn_reconf_f_ = boost::bind(&SampleHueDetectorRos::reconfigureCallback, this, _1, _2);
	dyn_reconf_srv_.setCallback(dyn_reconf_f_);

}

SampleHueDetectorRos::~SampleHueDetectorRos() {


}

void SampleHueDetectorRos::imageCallback(const sensor_msgs::ImageConstPtr& msg) {

	cv_bridge::CvImageConstPtr rgb;

	try {

		rgb = cv_bridge::toCvShare(msg, msg->encoding); // TODO use toCvShare

	}
	catch (cv_bridge::Exception& e)
	{
	  ROS_ERROR_THROTTLE(1.0, "Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
	  return;
	}

	ROS_INFO_ONCE("Received first RGB image.");

	if (pub_.getNumSubscribers() == 0) return;

	ROS_INFO_ONCE("Publishing first detection.");

	cv_bridge::CvImagePtr out_msg(new cv_bridge::CvImage);

	det_->detect(rgb,out_msg);

	pub_.publish(out_msg->toImageMsg());

}

void SampleHueDetectorRos::reconfigureCallback(rt_road_detection::SampleHueDetectorConfig &config, uint32_t level) {

	ROS_INFO("Reconfigure request.");

	// TODO check median ks size!!! + min < max etc
	// TODO add prob_hit / prob_miss

	det_->setParams(config.hue_min,config.hue_max,config.median_ks);

}
