﻿using System;
using System.Collections.Generic;
using System.Configuration;
using System.Diagnostics;
using System.Reflection;
using System.Windows.Forms;
using x360ce.App.Properties;
using x360ce.App.Win32;

namespace x360ce.App
{
	static class Program
	{

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			try
			{
				Application.EnableVisualStyles();
				Application.SetCompatibleTextRenderingDefault(false);
				// Requires System.Configuration.Installl reference.
				var ic = new System.Configuration.Install.InstallContext(null, args);
				if (ic.Parameters.ContainsKey("Settings"))
				{
					OpenSettingsFolder(Application.UserAppDataPath);
					OpenSettingsFolder(Application.CommonAppDataPath);
					OpenSettingsFolder(Application.LocalUserAppDataPath);
					return;
				}

				if (!CheckSettings()) return;
				//Application.ThreadException += new System.Threading.ThreadExceptionEventHandler(Application_ThreadException);
				MainForm.Current = new MainForm();
				if (ic.Parameters.ContainsKey("Exit"))
				{
					MainForm.Current.BroadcastMessage(MainForm.wParam_Close);
					return;
				}
				var ini = new Ini(SettingManager.IniFileName);
				var oneCopy = !ini.File.Exists || ini.GetValue("Options", SettingName.AllowOnlyOneCopy) == "1";
				if (!(oneCopy && MainForm.Current.BroadcastMessage(MainForm.wParam_Restore)))
				{
					Application.Run(MainForm.Current);
				}
			}
			catch (Exception ex)
			{
				var message = ex.ToString();
				AddLoaderException(ex, ref message);
				if (ex.InnerException != null) AddLoaderException(ex, ref message);
				var box = new Controls.MessageBoxForm();
				var result = box.ShowForm(message, "Exception!", MessageBoxButtons.RetryCancel, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
				if (result == DialogResult.Cancel) Application.Exit();
				throw ex;
			}
		}

		/// <summary>Add information about missing libraries and DLLs</summary>
		private static void AddLoaderException(Exception ex, ref string message)
		{
			if (!ex.GetType().Equals(typeof(ReflectionTypeLoadException))) return;
			message += "\r\n===============================================================\r\n";
			var exceptions = ((ReflectionTypeLoadException)ex).LoaderExceptions;
			foreach (Exception ex3 in exceptions) message += ex3.Message + "\r\n";
		}

		public static object DeviceLock = new object();

		public static int TimerCount = 0;
		public static int ReloadCount = 0;
		public static int ErrorCount = 0;

		public static void Application_ThreadException(object sender, System.Threading.ThreadExceptionEventArgs e)
		{
			ErrorCount++;
			MainForm.Current.UpdateTimer.Stop();
			MainForm.Current.UpdateStatus("- " + e.Exception.Message);
			MainForm.Current.UpdateTimer.Start();
		}

		static void OpenSettingsFolder(string path)
		{
			var di = new System.IO.DirectoryInfo(path);
			//if (!di.Exists) return;
			//if (di.GetFiles().Length == 0) return;
			var psi = new ProcessStartInfo(di.Parent.Parent.FullName);
			psi.UseShellExecute = true;
			psi.ErrorDialog = true;
			Process.Start(psi);
		}	

		static bool CheckSettings()
		{
			try
			{
				Settings.Default.Reload();
			}
			catch (ConfigurationErrorsException ex)
			{
				// Requires System.Configuration
				string filename = ((ConfigurationErrorsException)ex.InnerException).Filename;
				var title = "Corrupt user settings of " + Application.ProductName;
				var text =
					"Program has detected that your user settings file has become corrupted. " +
					"This may be due to a crash or improper exiting of the program. " +
					"Program must reset your user settings in order to continue.\r\n" +
					"Click [Yes] to reset your user settings and continue.\r\n" +
					"Click [No] if you wish to exit and attempt manual repair.";
				var result = MessageBox.Show(text, title, MessageBoxButtons.YesNo, MessageBoxIcon.Error);
				if (result == DialogResult.Yes)
				{
					System.IO.File.Delete(filename);
					Settings.Default.Reload();
				}
				else
				{
					OpenSettingsFolder(Application.UserAppDataPath);
					OpenSettingsFolder(Application.CommonAppDataPath);
					OpenSettingsFolder(Application.LocalUserAppDataPath);
					return false;
				}
			}
			return true;
		}

	}
}
